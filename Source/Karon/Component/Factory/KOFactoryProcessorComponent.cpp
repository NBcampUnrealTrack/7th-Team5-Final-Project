// Copyright Karon Team 5. All Rights Reserved.

#include "Component/Factory/KOFactoryProcessorComponent.h"

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

UKOFactoryProcessorComponent::UKOFactoryProcessorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
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
    
    // 레시피 미선택 / input이 아닌 아이템은 전부 거절
    if (!CanAcceptInputItemForSelectedRecipe(ItemId))
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
        BroadcastProcessorChanged();
        EvaluateAutoStart();
        
        if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
        {
            QuestGuide->NotifyProcessorInputInserted(ItemId, ToAdd);
        }
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

    BroadcastProcessorChanged();

    if (State == EKOFactoryState::OutputBlocked)
    {
        EvaluateAutoStart();
    }
    return Taken;
}

void UKOFactoryProcessorComponent::SetSelectedRecipe(FName RecipeId)
{
    if (SelectedRecipeId == RecipeId)
    {
        return;
    }
    SelectedRecipeId = RecipeId;
    BroadcastProcessorChanged();
    EvaluateAutoStart();
    
    // 퀘스트
    if (!SelectedRecipeId.IsNone())
    {
        if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
        {
            QuestGuide->NotifyRecipeSelected(SelectedRecipeId);
        }
    }
}

void UKOFactoryProcessorComponent::RestoreOutputBuffer(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return;
    }
    int32& Current = OutputBuffer.FindOrAdd(ItemId);
    Current += Count;
    BroadcastProcessorChanged();
}

int32 UKOFactoryProcessorComponent::TryExtractInputItem(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return 0;
    }

    int32* Found = InputBuffer.Find(ItemId);
    if (!Found || *Found <= 0)
    {
        return 0;
    }

    const int32 Taken = FMath::Min(*Found, Count);
    *Found -= Taken;
    if (*Found <= 0)
    {
        InputBuffer.Remove(ItemId);
    }
    if (Taken > 0)
    {
        BroadcastProcessorChanged();
    }
    return Taken;
}

void UKOFactoryProcessorComponent::RestoreInputBuffer(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return;
    }
    int32& Current = InputBuffer.FindOrAdd(ItemId);
    Current += Count;
    BroadcastProcessorChanged();
}

bool UKOFactoryProcessorComponent::ManualStart()
{
    if (!ActiveRecipeId.IsNone())
    {
        return false;
    }
    return TryStartCycle();
}

float UKOFactoryProcessorComponent::GetProgress() const
{
    if (ActiveRecipeId.IsNone() || CurrentCycleSeconds <= 0.f)
    {
        return 0.f;
    }
    return FMath::Clamp(Progress / CurrentCycleSeconds, 0.f, 1.f);
}

bool UKOFactoryProcessorComponent::HasAnyInputItems() const
{
    for (const TPair<FName, int32>& Pair : InputBuffer)
    {
        if (!Pair.Key.IsNone() && Pair.Value > 0)
        {
            return true;
        }
    }

    return false;
}

bool UKOFactoryProcessorComponent::HasAnyOutputItems() const
{
    for (const TPair<FName, int32>& Pair : OutputBuffer)
    {
        if (!Pair.Key.IsNone() && Pair.Value > 0)
        {
            return true;
        }
    }

    return false;
}

bool UKOFactoryProcessorComponent::CanChangeRecipe() const
{
    if (!ActiveRecipeId.IsNone())
    {
        return false;
    }

    if (State == EKOFactoryState::OutputBlocked)
    {
        return false;
    }

    if (HasAnyInputItems())
    {
        return false;
    }

    if (HasAnyOutputItems())
    {
        return false;
    }

    return true;
}

// IKOEnergyConsumer

float UKOFactoryProcessorComponent::GetPowerDemand(float DeltaSeconds) const
{
    if ((State != EKOFactoryState::Running && State != EKOFactoryState::PressureBlocked) || DeltaSeconds <= 0.f)
    {
        return 0.f;
    }
    const float PerSecond = GetDemandRecipePowerPerSecond();
    return PerSecond * DeltaSeconds;
}

void UKOFactoryProcessorComponent::OnPowerSupplied(float SuppliedAmount, float RequestedAmount)
{
    LastSupplyRatio = (RequestedAmount > KINDA_SMALL_NUMBER)
        ? FMath::Clamp(SuppliedAmount / RequestedAmount, 0.f, 1.f) : 1.f;
    
    AKOBaseBuilding* Building = GetOwnerBuilding();
    if (!Building)
    {
        return;
    }
    
    const bool bNetworkHasPressure = HasNetworkPressure();
    const bool bActuallySupplied = SuppliedAmount > KINDA_SMALL_NUMBER;
    const bool bActiveCycleRequiresPressure  = !ActiveRecipeId.IsNone() && RequestedAmount > KINDA_SMALL_NUMBER;
    const bool bPressureShortage = bActiveCycleRequiresPressure  && !bActuallySupplied;

    if (!bNetworkHasPressure)
    {
        LastSupplyRatio = 0.f;
        SetState(EKOFactoryState::PressureBlocked);
    }
    else if (State == EKOFactoryState::PressureBlocked)
    {
        if (!ActiveRecipeId.IsNone())
        {
            // 가공 도중 압력이 끊겼던 경우
            SetState(EKOFactoryState::Running);
        }
        else
        {
            // 가공 시작 전에 압력이 없었던 경우
            EvaluateAutoStart();
        }
    }

    const bool bActuallyOperating = State == EKOFactoryState::Running && bActuallySupplied;

    Building->SetOperatingSoundActive(bActuallyOperating);

    if (bPressureShortage && !bWasPressureShortage)
    {
        Building->PlayOperationBlockedSound();
    }

    bWasPressureShortage = bPressureShortage;
}

void UKOFactoryProcessorComponent::GetEnergyOccupiedCells(TArray<FIntPoint>& OutCells) const
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

    FIntPoint Anchor;
    FIntPoint Size;
    if (!Grid->TryGetOccupiedAreaForActor(Owner, Anchor, Size))
    {
        Anchor = Grid->WorldToGridPosition(Owner->GetActorLocation());
        Size = FIntPoint(1, 1);
    }

    OutCells.Reserve(Size.X * Size.Y);
    for (int32 Y = 0; Y < Size.Y; ++Y)
    {
        for (int32 X = 0; X < Size.X; ++X)
        {
            OutCells.Add(FIntPoint(Anchor.X + X, Anchor.Y + Y));
        }
    }
}

// IKOItemSource — 출력 버퍼의 첫 아이템을 벨트가 가져감.
bool UKOFactoryProcessorComponent::PeekOutputItem(FKOConveyorItem& OutItem) const
{
    for (const TPair<FName, int32>& Pair : OutputBuffer)
    {
        if (Pair.Value > 0)
        {
            OutItem = FKOConveyorItem(Pair.Key);
            return true;
        }
    }
    return false;
}

bool UKOFactoryProcessorComponent::PopOutputItem(FKOConveyorItem& OutItem)
{
    FKOConveyorItem Peeked;
    if (!PeekOutputItem(Peeked))
    {
        return false;
    }
    if (TryExtractItem(Peeked.ItemId, 1) == 1)
    {
        OutItem = Peeked;
        return true;
    }
    return false;
}

// IKOItemSink — 벨트가 입력 버퍼로 아이템을 밀어넣음. M1은 캡만 검사.
bool UKOFactoryProcessorComponent::CanAcceptItem(const FKOConveyorItem& Item) const
{
    if (!Item.IsValid())
    {
        return false;
    }

    if (!CanAcceptInputItemForSelectedRecipe(Item.ItemId))
    {
        return false;
    }

    const int32* Current = InputBuffer.Find(Item.ItemId);
    return (Current ? *Current : 0) < MaxBufferPerItem;
}

bool UKOFactoryProcessorComponent::PushItem(const FKOConveyorItem& Item)
{
    if (!Item.IsValid())
    {
        return false;
    }
    // TryInsertItem 은 받지 못한 잔여를 반환. 1개 전부 받았으면 잔여 0.
    return TryInsertItem(Item.ItemId, 1) == 0;
}

void UKOFactoryProcessorComponent::LoadProcessorStateFromSave(
    FName InSelectedRecipeId,
    const TMap<FName, int32>& InInputBuffer,
    const TMap<FName, int32>& InOutputBuffer,
    FName InActiveRecipeId,
    float InCurrentCycleSeconds,
    float InProgress
)
{
    SelectedRecipeId = InSelectedRecipeId;
    InputBuffer = InInputBuffer;
    OutputBuffer = InOutputBuffer;

    for (auto It = InputBuffer.CreateIterator(); It; ++It)
    {
        if (It.Key().IsNone() || It.Value() <= 0)
        {
            It.RemoveCurrent();
        }
    }

    for (auto It = OutputBuffer.CreateIterator(); It; ++It)
    {
        if (It.Key().IsNone() || It.Value() <= 0)
        {
            It.RemoveCurrent();
        }
    }

    if (!InActiveRecipeId.IsNone() && InCurrentCycleSeconds > KINDA_SMALL_NUMBER)
    {
        ActiveRecipeId = InActiveRecipeId;
        CurrentCycleSeconds = InCurrentCycleSeconds;
        Progress = FMath::Clamp(InProgress, 0.f, CurrentCycleSeconds);
        LastSupplyRatio = 0.f; // 로드 직후 압력/전력 공급을 다시 받을 때까지 진행 방지
        SetState(EKOFactoryState::Running);
    }
    else
    {
        ActiveRecipeId = NAME_None;
        CurrentCycleSeconds = 0.f;
        Progress = 0.f;
        LastSupplyRatio = 1.f;
        SetState(EKOFactoryState::Idle);
    }

    BroadcastProcessorChanged();
    BroadcastStateChanged();
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

    if (SelectedRecipeId.IsNone())
    {
        return NAME_None;
    }

    const FKORecipeRow* Selected = LoadSub->FindRecipeRow(SelectedRecipeId);
    if (!Selected || !Selected->AllowedFactoryTag.IsValid())
    {
        return NAME_None;
    }
    if (!MyRow->FactoryCategoryTag.MatchesTag(Selected->AllowedFactoryTag))
    {
        return NAME_None;
    }
    
    return SelectedRecipeId;
}

bool UKOFactoryProcessorComponent::TryStartCycle()
{
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    if (!LoadSub)
    {
        SetState(EKOFactoryState::Idle);
        return false;
    }

    const FName RecipeId = FindRunnableRecipe();
    if (RecipeId.IsNone())
    {
        SetState(EKOFactoryState::Idle);
        return false;
    }

    const FKORecipeRow* Recipe = LoadSub->FindRecipeRow(RecipeId);
    if (!Recipe)
    {
        SetState(EKOFactoryState::Idle);
        return false;
    }
    
    // 압력 검사
    if (!HasNetworkPressure())
    {
        LastSupplyRatio = 0.f;
        SetState(EKOFactoryState::PressureBlocked);
        return false;
    }

    // 재료(input) 검사
    if (!HasInputsFor(*Recipe))
    {
        SetState(EKOFactoryState::Idle);
        return false;
    }

    // 출력 공간(output) 검사
    if (!CanFitOutputs(*Recipe))
    {
        SetState(EKOFactoryState::OutputBlocked);
        return false;
    }
    
    // 재료 차감
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

    BroadcastProcessorChanged(); // InputBuffer 차감 + ActiveRecipe 변화
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
            const int32 ProducedCount = Out.Value;
            
            int32& Current = OutputBuffer.FindOrAdd(ItemId);
            Current += Out.Value;
            
            // 퀘스트
            if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
            {
                QuestGuide->NotifyItemCrafted(ItemId, ProducedCount);
            }
        }
    }

    ActiveRecipeId      = NAME_None;
    CurrentCycleSeconds = 0.f;
    Progress            = 0.f;

    BroadcastProcessorChanged(); // OutputBuffer 증가 + ActiveRecipe 해제
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

bool UKOFactoryProcessorComponent::HasNetworkPressure() const
{
    const UKOEnergySubsystem* Energy = UKOEnergySubsystem::Get(this);

    if (!Energy)
    {
        return false;
    }

    return Energy->GetConsumerNetworkProductionRate(this) > KINDA_SMALL_NUMBER;
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

float UKOFactoryProcessorComponent::GetDemandRecipePowerPerSecond() const
{
    FName RecipeId = NAME_None;

    // 가공 시작 상태
    if (!ActiveRecipeId.IsNone())
    {
        RecipeId = ActiveRecipeId;
    }
    // 시작 전 압력 부족
    else if (State == EKOFactoryState::PressureBlocked)
    {
        RecipeId = SelectedRecipeId;
    }

    if (RecipeId.IsNone())
    {
        return 0.f;
    }

    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    const FKORecipeRow* Recipe = LoadSub ? LoadSub->FindRecipeRow(RecipeId) : nullptr;

    return Recipe ? FMath::Max(0.f, Recipe->PowerPerSecond) : 0.f;
}

bool UKOFactoryProcessorComponent::CanAcceptInputItemForSelectedRecipe(FName ItemId) const
{
    if (ItemId.IsNone())
    {
        return false;
    }

    // 레시피가 선택되지 않은 설비는 어떤 input도 받지 않는다.
    if (SelectedRecipeId.IsNone())
    {
        return false;
    }
    
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    if (!LoadSub)
    {
        return false;
    }

    const FKORecipeRow* Selected = LoadSub->FindRecipeRow(SelectedRecipeId);
    if (!Selected)
    {
        return false;
    }

    // 선택된 레시피의 Inputs에 포함된 아이템인지 확인한다.
    for (const TPair<FGameplayTag, int32>& In : Selected->Inputs)
    {
        const FName RequiredItemId = LoadSub->FindItemIdByTag(In.Key);
        if (RequiredItemId == ItemId)
        {
            return true;
        }
    }

    return false;
}

void UKOFactoryProcessorComponent::EvaluateAutoStart()
{
    if (!bAutoStart || !ActiveRecipeId.IsNone())
    {
        return;
    }
    
    // 압력 검사
    if (!HasNetworkPressure())
    {
        LastSupplyRatio = 0.f;
        SetState(EKOFactoryState::PressureBlocked);
        return;
    }

    TryStartCycle();
}

void UKOFactoryProcessorComponent::SetState(EKOFactoryState NewState)
{
    if (State == NewState)
    {
        SetComponentTickEnabled(State == EKOFactoryState::Running);
        
        if (State != EKOFactoryState::Running)
        {
            if (AKOBaseBuilding* Building = GetOwnerBuilding())
            {
                Building->SetOperatingSoundActive(false);
            }
        }
        return;
    }
    
    State = NewState;
    const bool bIsRunning = State == EKOFactoryState::Running;
    // 제작 중에만 Tick 활성화
    SetComponentTickEnabled(bIsRunning);

    if (!bIsRunning)
    {
        // 재료 부족, 레시피 없음, 출력 막힘
        // 모두 경고음 없이 가동음만 정지
        if (AKOBaseBuilding* Building = GetOwnerBuilding())
        {
            Building->SetOperatingSoundActive(false);
        }
    }
    BroadcastStateChanged();
    BroadcastProcessorChanged();
}

void UKOFactoryProcessorComponent::BroadcastProcessorChanged() const
{
    const UWorld* World = GetWorld();
    if (!World) return;
    UGameInstance* GI = World->GetGameInstance();
    if (!GI) return;
    UGMRouterSubsystem* GMS = GI->GetSubsystem<UGMRouterSubsystem>();
    if (!GMS) return;

    FKOProcessorChangedMessage Msg;
    Msg.Processor = const_cast<UKOFactoryProcessorComponent*>(this);
    GMS->BroadcastMessage(
        KOGameplayTags::Data_Message_Processor_Changed,
        FInstancedStruct::Make(Msg));
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
