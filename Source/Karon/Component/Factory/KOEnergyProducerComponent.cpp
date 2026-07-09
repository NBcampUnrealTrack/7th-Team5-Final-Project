// Copyright Karon Team 5. All Rights Reserved.

#include "Component/Factory/KOEnergyProducerComponent.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Building/KOBaseBuilding.h"
#include "Data/KODataTableTypes.h"
#include "GMRouterSubsystem.h"


#include "StructUtils/InstancedStruct.h"
#include "Subsystem/KOEnergySubsystem.h"
#include "Subsystem/KOGridSubsystem.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Subsystem/KOQuestGuideSubsystem.h"
#include "Utility/Messaging/KOMessageTypes.h"

UKOEnergyProducerComponent::UKOEnergyProducerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UKOEnergyProducerComponent::InitializeFromRecipe()
{
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    if (!LoadSub) return;

    const AKOBaseBuilding* Building = Cast<AKOBaseBuilding>(GetOwner());
    if (!Building) return;

    const FKOFactoryRow* FactoryRow = Building->GetFactoryRow();
    if (!FactoryRow || !FactoryRow->FactoryCategoryTag.IsValid()) return;

    TArray<FName> AllRecipes;
    LoadSub->GetAllRecipeIds(AllRecipes);

    for (const FName& Id : AllRecipes)
    {
        const FKORecipeRow* Recipe = LoadSub->FindRecipeRow(Id);
        if (!Recipe || !Recipe->AllowedFactoryTag.IsValid()) continue;
        if (!FactoryRow->FactoryCategoryTag.MatchesTag(Recipe->AllowedFactoryTag)) continue;

        RecipeId = Id;

        const float CycleSeconds = FMath::Max(Recipe->CycleSeconds, KINDA_SMALL_NUMBER);
        BurnRatePerSecond = 1.f / CycleSeconds;
        PowerPerFuelUnit = Recipe->PowerPerSecond * CycleSeconds;

        for (const TPair<FGameplayTag, int32>& Input : Recipe->Inputs)
        {
            AcceptedFuelItemId = LoadSub->FindItemIdByTag(Input.Key);
            break;
        }

        UE_LOG(LogTemp, Log, TEXT("[Producer] Recipe '%s': BurnRate=%.2f/s, PowerPerFuel=%.1f, Fuel='%s'"),
            *RecipeId.ToString(), BurnRatePerSecond, PowerPerFuelUnit, *AcceptedFuelItemId.ToString());
        return;
    }
}

void UKOEnergyProducerComponent::LoadFuelFromSave(
    FName InFuelItemId, int32 InFuelCount, float InFuelDebt, bool bInHasActiveFuel)
{
    FuelItemId = InFuelItemId;
    FuelInBuffer = FMath::Max(0, InFuelCount);
    bHasActiveFuel = bInHasActiveFuel && !FuelItemId.IsNone();

    if (bHasActiveFuel)
    {
        FuelDebt = FMath::Clamp(InFuelDebt, 0.f, 0.999f);
    }
    else
    {
        FuelDebt = 0.f;
    }

    if (FuelItemId.IsNone())
    {
        FuelInBuffer = 0;
        FuelDebt = 0.f;
        bHasActiveFuel = false;
    }

    if (!bHasActiveFuel && FuelInBuffer <= 0)
    {
        FuelInBuffer = 0;
        FuelDebt = 0.f;
        FuelItemId = NAME_None;
    }

    BroadcastFuelChanged();
}

void UKOEnergyProducerComponent::BeginPlay()
{
    Super::BeginPlay();
    if (UKOEnergySubsystem* Energy = UKOEnergySubsystem::Get(this))
    {
        Energy->RegisterProducer(this);
    }
}

void UKOEnergyProducerComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if (UKOEnergySubsystem* Energy = UKOEnergySubsystem::Get(this))
    {
        Energy->UnregisterProducer(this);
    }
    Super::EndPlay(Reason);
}

int32 UKOEnergyProducerComponent::TryInsertFuel(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return Count;
    }

    if (AcceptedFuelItemId.IsNone() || ItemId != AcceptedFuelItemId)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Producer] TryInsertFuel REJECTED: ItemId='%s', AcceptedFuelItemId='%s', RecipeId='%s'"),
            *ItemId.ToString(), *AcceptedFuelItemId.ToString(), *RecipeId.ToString());
        return Count;
    }

    const int32 CurrentTotalFuel = FuelInBuffer + (bHasActiveFuel ? 1 : 0);
    const int32 Space = FMath::Max(0, MaxFuelBuffer - CurrentTotalFuel);
    const int32 ToAdd = FMath::Min(Space, Count);
    if (ToAdd > 0)
    {
        FuelInBuffer += ToAdd;
        FuelItemId    = ItemId;
        StartNextFuelIfNeeded();
        BroadcastFuelChanged();
        
        // 퀘스트
        if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
        {
            QuestGuide->NotifyFuelInserted(ItemId, ToAdd);
        }
    }
    return Count - ToAdd;
}

int32 UKOEnergyProducerComponent::TryExtractFuel(int32 Count)
{
    if (Count <= 0 || FuelInBuffer <= 0 || FuelItemId.IsNone())
    {
        return 0;
    }
    const int32 Taken = FMath::Min(FuelInBuffer, Count);
    FuelInBuffer -= Taken;
    if (FuelInBuffer <= 0 && !bHasActiveFuel)
    {
        FuelInBuffer = 0;
        FuelDebt     = 0.f;
        FuelItemId   = NAME_None;
    }
    if (Taken > 0)
    {
        BroadcastFuelChanged();
    }
    return Taken;
}

void UKOEnergyProducerComponent::RestoreFuelBuffer(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return;
    }
    FuelInBuffer += Count;
    FuelItemId    = ItemId;
    BroadcastFuelChanged();
}

float UKOEnergyProducerComponent::GetPowerOutput(float DeltaSeconds) const
{
    if (DeltaSeconds <= 0.f || PowerPerFuelUnit <= 0.f || BurnRatePerSecond <= 0.f)
    {
        return 0.f;
    }
    
    if (!bHasActiveFuel)
    {
        return 0.f;
    }
    
    const float RemainingFuelRatio = 1.f - FuelDebt;
    if (RemainingFuelRatio <= 0.f)
    {
        return 0.f;
    }

    const float DesiredBurn = BurnRatePerSecond * DeltaSeconds;
    const float ActualBurn  = FMath::Min(DesiredBurn, RemainingFuelRatio);
    return ActualBurn * PowerPerFuelUnit;
}

void UKOEnergyProducerComponent::OnPowerAccepted(float Amount)
{
    // 매 틱(공급 0 포함) 호출되도록 서브시스템이 보장 → 여기서 초당 출력 캐싱.
    const UWorld* World = GetWorld();
    const float Dt = World ? World->GetDeltaSeconds() : 0.f;
    LastOutputRate = (Dt > KINDA_SMALL_NUMBER) ? (FMath::Max(0.f, Amount) / Dt) : 0.f;

    if (Amount <= 0.f || PowerPerFuelUnit <= 0.f || !bHasActiveFuel)
    {
        return;
    }

    const float FuelConsumed = Amount / PowerPerFuelUnit;
    FuelDebt += FuelConsumed;
    
    if (FuelDebt >= 1.f - KINDA_SMALL_NUMBER)
    {
        FuelDebt = 0.f;
        bHasActiveFuel = false;

        StartNextFuelIfNeeded();

        if (!bHasActiveFuel && FuelInBuffer <= 0)
        {
            FuelItemId = NAME_None;
        }

        BroadcastFuelChanged();
    }
}

void UKOEnergyProducerComponent::GetEnergyCoverageCells(TArray<FIntPoint>& OutCells) const
{
    OutCells.Reset();

    AActor* Owner = GetOwner();
    const UWorld* World = GetWorld();
    if (!Owner || !World)
    {
        return;
    }

    UKOGridSubsystem* Grid = World->GetSubsystem<UKOGridSubsystem>();
    if (!Grid)
    {
        return;
    }

    // 점유 영역을 우선 사용. 그리드 미등록(에디터 선배치 등)이면 월드 위치로 폴백.
    FIntPoint Anchor;
    FIntPoint Size;
    if (!Grid->TryGetOccupiedAreaForActor(Owner, Anchor, Size))
    {
        Anchor = Grid->WorldToGridPosition(Owner->GetActorLocation());
        Size = FIntPoint(1, 1);
    }

    int32 Radius = 0;
    if (const AKOBaseBuilding* Building = Cast<AKOBaseBuilding>(Owner))
    {
        if (const FKOFactoryRow* Row = Building->GetFactoryRow())
        {
            Radius = FMath::Max(0, Row->EnergyCoverageRadius);
        }
    }

    const FIntPoint Start(Anchor.X - Radius, Anchor.Y - Radius);
    const FIntPoint Extent(Size.X + 2 * Radius, Size.Y + 2 * Radius);

    OutCells.Reserve(Extent.X * Extent.Y);
    for (int32 Y = 0; Y < Extent.Y; ++Y)
    {
        for (int32 X = 0; X < Extent.X; ++X)
        {
            OutCells.Add(FIntPoint(Start.X + X, Start.Y + Y));
        }
    }
}

// IKOItemSink — 연료 카테고리/버퍼/혼합 여부를 TryInsertFuel 과 동일 규칙으로 검사.
bool UKOEnergyProducerComponent::CanAcceptItem(const FKOConveyorItem& Item) const
{
    if (!Item.IsValid() || AcceptedFuelItemId.IsNone())
    {
        return false;
    }
    if (Item.ItemId != AcceptedFuelItemId)
    {
        return false;
    }
    const int32 CurrentTotalFuel = FuelInBuffer + (bHasActiveFuel ? 1 : 0);
    return CurrentTotalFuel < MaxFuelBuffer;
}

bool UKOEnergyProducerComponent::PushItem(const FKOConveyorItem& Item)
{
    if (!Item.IsValid())
    {
        return false;
    }
    return TryInsertFuel(Item.ItemId, 1) == 0;
}

void UKOEnergyProducerComponent::BroadcastFuelChanged() const
{
    const UWorld* World = GetWorld();
    if (!World) return;
    UGameInstance* GI = World->GetGameInstance();
    if (!GI) return;
    UGMRouterSubsystem* GMS = GI->GetSubsystem<UGMRouterSubsystem>();
    if (!GMS) return;

    FKOProducerFuelChangedMessage Msg;
    Msg.Producer   = const_cast<UKOEnergyProducerComponent*>(this);
    Msg.FuelItemId = FuelItemId;
    Msg.FuelCount  = FuelInBuffer;
    GMS->BroadcastMessage(
        KOGameplayTags::Data_Message_Producer_FuelChanged,
        FInstancedStruct::Make(Msg));
}

void UKOEnergyProducerComponent::StartNextFuelIfNeeded()
{
    if (bHasActiveFuel)
    {
        return;
    }

    if (FuelInBuffer <= 0 || FuelItemId.IsNone())
    {
        bHasActiveFuel = false;
        FuelDebt = 0.f;
        return;
    }

    FuelInBuffer -= 1;
    FuelDebt = 0.f;
    bHasActiveFuel = true;
}
