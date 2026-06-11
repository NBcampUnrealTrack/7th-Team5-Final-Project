// Copyright Karon Team 5. All Rights Reserved.

#include "Component/Factory/KOEnergyProducerComponent.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Data/KODataTableTypes.h"
#include "GMRouterSubsystem.h"


#include "StructUtils/InstancedStruct.h"
#include "Subsystem/KOEnergySubsystem.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Utility/Messaging/KOMessageTypes.h"

UKOEnergyProducerComponent::UKOEnergyProducerComponent()
    : FuelCategoryTag(KOGameplayTags::Item_Category_EnergyResource)
{
    PrimaryComponentTick.bCanEverTick = false;
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

    if (!FuelCategoryTag.IsValid())
    {
        return Count;
    }

    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    const FKOItemRow* Row = LoadSub ? LoadSub->FindItemRow(ItemId) : nullptr;
    if (!Row || !Row->Categories.HasTag(FuelCategoryTag))
    {
        return Count;
    }

    const int32 Space = FMath::Max(0, MaxFuelBuffer - FuelInBuffer);
    const int32 ToAdd = FMath::Min(Space, Count);
    if (ToAdd > 0)
    {
        FuelInBuffer += ToAdd;
        FuelItemId    = ItemId;
        BroadcastFuelChanged();
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
    if (FuelInBuffer <= 0)
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
    
    const float AvailableFuelUnits = static_cast<float>(FuelInBuffer) - FuelDebt;
    if (AvailableFuelUnits <= 0.f)
    {
        return 0.f;
    }

    const float DesiredBurn = BurnRatePerSecond * DeltaSeconds;
    const float ActualBurn  = FMath::Min(DesiredBurn, AvailableFuelUnits);
    return ActualBurn * PowerPerFuelUnit;
}

void UKOEnergyProducerComponent::OnPowerAccepted(float Amount)
{
    if (Amount <= 0.f || PowerPerFuelUnit <= 0.f)
    {
        return;
    }

    const float FuelConsumed = Amount / PowerPerFuelUnit;
    FuelDebt += FuelConsumed;
    
    const int32 WholeUnits = FMath::FloorToInt(FuelDebt);
    if (WholeUnits > 0)
    {
        const int32 ToRemove = FMath::Min(WholeUnits, FuelInBuffer);
        FuelInBuffer -= ToRemove;
        FuelDebt     -= static_cast<float>(ToRemove);
    }

    if (FuelInBuffer <= 0)
    {
        const bool bWasFueled = !FuelItemId.IsNone();
        FuelInBuffer = 0;
        FuelDebt     = 0.f;
        FuelItemId   = NAME_None;
        if (bWasFueled)
        {
            BroadcastFuelChanged();
        }
    }
}

// IKOItemSink — 연료 카테고리/버퍼/혼합 여부를 TryInsertFuel 과 동일 규칙으로 검사.
bool UKOEnergyProducerComponent::CanAcceptItem(const FKOConveyorItem& Item) const
{
    if (!Item.IsValid() || !FuelCategoryTag.IsValid())
    {
        return false;
    }
    // 이미 다른 연료가 적재돼 있으면 혼합 불가.
    if (!FuelItemId.IsNone() && FuelItemId != Item.ItemId)
    {
        return false;
    }
    if (FuelInBuffer >= MaxFuelBuffer)
    {
        return false;
    }
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    const FKOItemRow* Row = LoadSub ? LoadSub->FindItemRow(Item.ItemId) : nullptr;
    return Row && Row->Categories.HasTag(FuelCategoryTag);
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
