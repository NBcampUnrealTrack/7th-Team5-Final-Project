// Copyright Karon Team 5. All Rights Reserved.

#include "Component/KOFactoryProcessorComponent.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Building/KOBaseBuilding.h"
#include "Data/KODataTableTypes.h"
#include "GMRouterSubsystem.h"
#include "Messaging/KOMessageTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystem/KOEnergySubsystem.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UKOFactoryProcessorComponent::UKOFactoryProcessorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UKOFactoryProcessorComponent::BeginPlay()
{
    Super::BeginPlay();

    if (UKOEnergySubsystem* Energy = UKOEnergySubsystem::Get(this))
    {
        Energy->RegisterConsumer(this);
    }

    EvaluateAutoStart();
}

void UKOFactoryProcessorComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if (UKOEnergySubsystem* Energy = UKOEnergySubsystem::Get(this))
    {
        Energy->UnregisterConsumer(this);
    }
    Super::EndPlay(Reason);
}

void UKOFactoryProcessorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (State != EKOFactoryState::Running || DeltaTime <= 0.f)
    {
        return;
    }

    Progress += DeltaTime * LastSupplyRatio;
    if (Progress >= CurrentCycleSeconds)
    {
        OnCycleComplete();
    }
}


int32 UKOFactoryProcessorComponent::TryInsertItem(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return Count;
    }

    int32& Current = InputBuffer.FindOrAdd(ItemId);
    const int32 Space = FMath::Max(0, MaxBufferPerItem - Current);
    const int32 ToAdd = FMath::Min(Space, Count);

    Current += ToAdd;
    const int32 Remaining = Count - ToAdd;

    if (ToAdd > 0)
    {
        EvaluateAutoStart();
    }
    return Remaining;
}

int32 UKOFactoryProcessorComponent::TryExtractItem(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return 0;
    }

    int32* Found = OutputBuffer.Find(ItemId);
    if (!Found || *Found <= 0)
    {
        return 0;
    }

    const int32 Taken = FMath::Min(*Found, Count);
    *Found -= Taken;
    if (*Found <= 0)
    {
        OutputBuffer.Remove(ItemId);
    }

    if (State == EKOFactoryState::OutputBlocked)
    {
        EvaluateAutoStart();
    }
    return Taken;
}

void UKOFactoryProcessorComponent::RestoreOutputBuffer(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return;
    }
    int32& Current = OutputBuffer.FindOrAdd(ItemId);
    Current += Count;
}

bool UKOFactoryProcessorComponent::ManualStart()
{
    if (State != EKOFactoryState::Idle)
    {
        return false;
    }
    return TryStartCycle();
}

float UKOFactoryProcessorComponent::GetProgress() const
{
    if (State != EKOFactoryState::Running || CurrentCycleSeconds <= 0.f)
    {
        return 0.f;
    }
    return FMath::Clamp(Progress / CurrentCycleSeconds, 0.f, 1.f);
}

// IKOEnergyConsumer

float UKOFactoryProcessorComponent::GetPowerDemand(float DeltaSeconds) const
{
    if (State != EKOFactoryState::Running || DeltaSeconds <= 0.f)
    {
        return 0.f;
    }
    const float PerSecond = GetActiveRecipePowerPerSecond();
    return PerSecond * DeltaSeconds;
}

void UKOFactoryProcessorComponent::OnPowerSupplied(float SuppliedAmount, float RequestedAmount)
{
    LastSupplyRatio = (RequestedAmount > KINDA_SMALL_NUMBER)
        ? FMath::Clamp(SuppliedAmount / RequestedAmount, 0.f, 1.f)
        : 1.f;
}

// Internal Function
FName UKOFactoryProcessorComponent::FindRunnableRecipe() const
{
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    if (!LoadSub)
    {
        return NAME_None;
    }

    const FName FactoryId = GetOwnerFactoryId();
    if (FactoryId.IsNone())
    {
        return NAME_None;
    }
    const FKOFactoryRow* MyRow = LoadSub->FindFactoryRow(FactoryId);
    if (!MyRow || !MyRow->FactoryCategoryTag.IsValid())
    {
        return NAME_None;
    }

    TArray<FName> AllRecipes;
    LoadSub->GetAllRecipeIds(AllRecipes);

    for (const FName& RecipeId : AllRecipes)
    {
        const FKORecipeRow* Recipe = LoadSub->FindRecipeRow(RecipeId);
        if (!Recipe || !Recipe->AllowedFactoryTag.IsValid())
        {
            continue;
        }
        if (!MyRow->FactoryCategoryTag.MatchesTag(Recipe->AllowedFactoryTag))
        {
            continue;
        }
        if (HasInputsFor(*Recipe) && CanFitOutputs(*Recipe))
        {
            return RecipeId;
        }
    }
    return NAME_None;
}

bool UKOFactoryProcessorComponent::TryStartCycle()
{
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    if (!LoadSub)
    {
        return false;
    }

    const FName RecipeId = FindRunnableRecipe();
    if (RecipeId.IsNone())
    {
        return false;
    }

    const FKORecipeRow* Recipe = LoadSub->FindRecipeRow(RecipeId);
    if (!Recipe)
    {
        return false;
    }
    
    for (const TPair<FGameplayTag, int32>& In : Recipe->Inputs)
    {
        const FName ItemId = LoadSub->FindItemIdByTag(In.Key);
        if (ItemId.IsNone())
        {
            return false;
        }
        int32* Have = InputBuffer.Find(ItemId);
        if (!Have)
        {
            return false;
        }
        *Have -= In.Value;
        if (*Have <= 0)
        {
            InputBuffer.Remove(ItemId);
        }
    }

    ActiveRecipeId      = RecipeId;
    CurrentCycleSeconds = FMath::Max(0.f, Recipe->CycleSeconds);
    Progress            = 0.f;

    SetState(EKOFactoryState::Running);
    
    if (CurrentCycleSeconds <= KINDA_SMALL_NUMBER)
    {
        OnCycleComplete();
    }
    return true;
}

void UKOFactoryProcessorComponent::OnCycleComplete()
{
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    const FKORecipeRow* Recipe = LoadSub ? LoadSub->FindRecipeRow(ActiveRecipeId) : nullptr;

    if (Recipe)
    {
        for (const TPair<FGameplayTag, int32>& Out : Recipe->Outputs)
        {
            const FName ItemId = LoadSub ? LoadSub->FindItemIdByTag(Out.Key) : NAME_None;
            if (ItemId.IsNone())
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("[Factory] OnCycleComplete: ItemTag '%s' 해석 실패 — 출력 누락"),
                    *Out.Key.ToString());
                continue;
            }
            int32& Current = OutputBuffer.FindOrAdd(ItemId);
            Current += Out.Value;
        }
    }

    ActiveRecipeId      = NAME_None;
    CurrentCycleSeconds = 0.f;
    Progress            = 0.f;

    SetState(EKOFactoryState::Idle);
    EvaluateAutoStart();
}

bool UKOFactoryProcessorComponent::HasInputsFor(const FKORecipeRow& Recipe) const
{
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    if (!LoadSub)
    {
        return false;
    }

    for (const TPair<FGameplayTag, int32>& In : Recipe.Inputs)
    {
        const FName ItemId = LoadSub->FindItemIdByTag(In.Key);
        if (ItemId.IsNone())
        {
            return false;
        }
        const int32* Have = InputBuffer.Find(ItemId);
        if (!Have || *Have < In.Value)
        {
            return false;
        }
    }
    return true;
}

bool UKOFactoryProcessorComponent::CanFitOutputs(const FKORecipeRow& Recipe) const
{
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    if (!LoadSub)
    {
        return false;
    }

    for (const TPair<FGameplayTag, int32>& Out : Recipe.Outputs)
    {
        const FName ItemId = LoadSub->FindItemIdByTag(Out.Key);
        if (ItemId.IsNone())
        {
            return false;
        }
        const int32* Current = OutputBuffer.Find(ItemId);
        const int32 After = (Current ? *Current : 0) + Out.Value;
        if (After > MaxBufferPerItem)
        {
            return false;
        }
    }
    return true;
}

float UKOFactoryProcessorComponent::GetActiveRecipePowerPerSecond() const
{
    if (ActiveRecipeId.IsNone())
    {
        return 0.f;
    }
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    const FKORecipeRow* Recipe = LoadSub ? LoadSub->FindRecipeRow(ActiveRecipeId) : nullptr;
    return Recipe ? FMath::Max(0.f, Recipe->PowerPerSecond) : 0.f;
}

void UKOFactoryProcessorComponent::EvaluateAutoStart()
{
    if (!bAutoStart || State == EKOFactoryState::Running)
    {
        return;
    }

    if (TryStartCycle())
    {
        return;
    }
    
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    if (!LoadSub)
    {
        return;
    }
    const FName FactoryId = GetOwnerFactoryId();
    if (FactoryId.IsNone())
    {
        return;
    }
    const FKOFactoryRow* MyRow = LoadSub->FindFactoryRow(FactoryId);
    if (!MyRow || !MyRow->FactoryCategoryTag.IsValid())
    {
        return;
    }

    TArray<FName> AllRecipes;
    LoadSub->GetAllRecipeIds(AllRecipes);
    bool bAnyOutputBlocked = false;
    for (const FName& RecipeId : AllRecipes)
    {
        const FKORecipeRow* Recipe = LoadSub->FindRecipeRow(RecipeId);
        if (!Recipe || !Recipe->AllowedFactoryTag.IsValid())
        {
            continue;
        }
        if (!MyRow->FactoryCategoryTag.MatchesTag(Recipe->AllowedFactoryTag))
        {
            continue;
        }
        if (HasInputsFor(*Recipe) && !CanFitOutputs(*Recipe))
        {
            bAnyOutputBlocked = true;
            break;
        }
    }
    SetState(bAnyOutputBlocked ? EKOFactoryState::OutputBlocked : EKOFactoryState::Idle);
}

void UKOFactoryProcessorComponent::SetState(EKOFactoryState NewState)
{
    if (State == NewState)
    {
        return;
    }
    State = NewState;
    BroadcastStateChanged();
}

void UKOFactoryProcessorComponent::BroadcastStateChanged() const
{
    const UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
    {
        return;
    }
    UGMRouterSubsystem* GMS = GI->GetSubsystem<UGMRouterSubsystem>();
    if (!GMS)
    {
        return;
    }

    FKOFactoryStateChangedMessage Msg;
    Msg.FactoryId = GetOwnerFactoryId();
    Msg.bIsActive = (State == EKOFactoryState::Running);
    Msg.Progress  = GetProgress();

    GMS->BroadcastMessage(
        KOGameplayTags::Data_Message_Factory_StateChanged,
        FInstancedStruct::Make(Msg));
}

AKOBaseBuilding* UKOFactoryProcessorComponent::GetOwnerBuilding() const
{
    return Cast<AKOBaseBuilding>(GetOwner());
}

FName UKOFactoryProcessorComponent::GetOwnerFactoryId() const
{
    const AKOBaseBuilding* Building = GetOwnerBuilding();
    return Building ? Building->GetFactoryId() : NAME_None;
}
