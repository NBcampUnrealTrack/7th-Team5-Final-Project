// Copyright Karon Team 5. All Rights Reserved.

#include "Component/KOFactoryProcessorComponent.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Building/KOBaseBuilding.h"
#include "Data/KODataTableTypes.h"
#include "GMRouterSubsystem.h"
#include "Messaging/KOMessageTypes.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "TimerManager.h"

UKOFactoryProcessorComponent::UKOFactoryProcessorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UKOFactoryProcessorComponent::BeginPlay()
{
    Super::BeginPlay();
    EvaluateAutoStart();
}

void UKOFactoryProcessorComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(CycleTimer);
    }
    Super::EndPlay(Reason);
}

// ─── 외부 API ────────────────────────────────────────────────────────────────

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

    // 출력이 비워졌으니 OutputBlocked였다면 재가동 시도
    if (State == EKOFactoryState::OutputBlocked)
    {
        EvaluateAutoStart();
    }
    return Taken;
}

bool UKOFactoryProcessorComponent::ManualStart()
{
    if (State != EKOFactoryState::Idle)
    {
        return false;
    }
    return TryStartCycle();
}

float UKOFactoryProcessorComponent::GetProgress01() const
{
    if (State != EKOFactoryState::Running || CurrentCycleSeconds <= 0.f)
    {
        return 0.f;
    }

    const UWorld* World = GetWorld();
    if (!World)
    {
        return 0.f;
    }

    const double Elapsed = World->GetTimeSeconds() - CycleStartSeconds;
    return FMath::Clamp(static_cast<float>(Elapsed) / CurrentCycleSeconds, 0.f, 1.f);
}

// ─── 내부 동작 ───────────────────────────────────────────────────────────────

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

    TArray<FName> AllRecipes;
    LoadSub->GetAllRecipeIds(AllRecipes);

    for (const FName& RecipeId : AllRecipes)
    {
        const FKORecipeRow* Recipe = LoadSub->FindRecipeRow(RecipeId);
        if (!Recipe)
        {
            continue;
        }
        if (!Recipe->AllowedFactoryIds.Contains(FactoryId))
        {
            continue;
        }
        if (!HasInputsFor(*Recipe))
        {
            continue;
        }
        if (!CanFitOutputs(*Recipe))
        {
            continue;
        }
        return RecipeId;
    }
    return NAME_None;
}

bool UKOFactoryProcessorComponent::TryStartCycle()
{
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    UWorld* World = GetWorld();
    if (!LoadSub || !World)
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

    // 입력 즉시 차감 (사이클 중 다른 가공이 입력을 가로채는 것 방지)
    for (const TPair<FName, int32>& In : Recipe->Inputs)
    {
        int32* Have = InputBuffer.Find(In.Key);
        if (!Have)
        {
            // 사실 HasInputsFor에서 보장되지만 방어
            return false;
        }
        *Have -= In.Value;
        if (*Have <= 0)
        {
            InputBuffer.Remove(In.Key);
        }
    }

    ActiveRecipeId      = RecipeId;
    CurrentCycleSeconds = FMath::Max(0.f, Recipe->CycleSeconds);
    CycleStartSeconds   = World->GetTimeSeconds();

    SetState(EKOFactoryState::Running);

    if (CurrentCycleSeconds <= KINDA_SMALL_NUMBER)
    {
        // 0초 레시피(예: 즉시 가공)는 즉시 완료
        OnCycleComplete();
    }
    else
    {
        World->GetTimerManager().SetTimer(
            CycleTimer,
            FTimerDelegate::CreateUObject(this, &UKOFactoryProcessorComponent::OnCycleComplete),
            CurrentCycleSeconds,
            /*bLoop*/ false);
    }
    return true;
}

void UKOFactoryProcessorComponent::OnCycleComplete()
{
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    const FKORecipeRow* Recipe = LoadSub ? LoadSub->FindRecipeRow(ActiveRecipeId) : nullptr;

    if (Recipe)
    {
        for (const TPair<FName, int32>& Out : Recipe->Outputs)
        {
            int32& Current = OutputBuffer.FindOrAdd(Out.Key);
            Current += Out.Value;
        }
    }

    ActiveRecipeId      = NAME_None;
    CurrentCycleSeconds = 0.f;

    SetState(EKOFactoryState::Idle);
    EvaluateAutoStart();
}

bool UKOFactoryProcessorComponent::HasInputsFor(const FKORecipeRow& Recipe) const
{
    for (const TPair<FName, int32>& In : Recipe.Inputs)
    {
        const int32* Have = InputBuffer.Find(In.Key);
        if (!Have || *Have < In.Value)
        {
            return false;
        }
    }
    return true;
}

bool UKOFactoryProcessorComponent::CanFitOutputs(const FKORecipeRow& Recipe) const
{
    for (const TPair<FName, int32>& Out : Recipe.Outputs)
    {
        const int32* Current = OutputBuffer.Find(Out.Key);
        const int32 After = (Current ? *Current : 0) + Out.Value;
        if (After > MaxBufferPerItem)
        {
            return false;
        }
    }
    return true;
}

void UKOFactoryProcessorComponent::EvaluateAutoStart()
{
    if (!bAutoStart)
    {
        return;
    }
    if (State == EKOFactoryState::Running)
    {
        return;
    }

    // OutputBlocked일 때는 출력 여유 확보된 경우에만 진행
    if (TryStartCycle())
    {
        return;
    }

    // 시작 못 했다면 사유에 따라 상태 갱신
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

    // 입력은 있으나 출력이 막혀 못 시작한 경우 → OutputBlocked
    TArray<FName> AllRecipes;
    LoadSub->GetAllRecipeIds(AllRecipes);
    bool bAnyOutputBlocked = false;
    for (const FName& RecipeId : AllRecipes)
    {
        const FKORecipeRow* Recipe = LoadSub->FindRecipeRow(RecipeId);
        if (!Recipe || !Recipe->AllowedFactoryIds.Contains(FactoryId))
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
    Msg.Progress  = GetProgress01();

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
