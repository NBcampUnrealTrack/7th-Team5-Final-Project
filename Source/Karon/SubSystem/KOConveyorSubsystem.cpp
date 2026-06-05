// Copyright Karon Team 5. All Rights Reserved.

#include "Subsystem/KOConveyorSubsystem.h"

#include "Building/Conveyor/KOConveyorBelt.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogKOConveyor, Log, All);

// 진단 스탯: 등록 벨트 수 / 이동 중 아이템 수(매 틱 SET) / 일괄 틱 사이클.
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Conveyor Belts"), STAT_KOConveyorBelts, STATGROUP_Tickables);
DECLARE_DWORD_ACCUMULATOR_STAT(TEXT("Conveyor Items"), STAT_KOConveyorItems, STATGROUP_Tickables);
DECLARE_CYCLE_STAT(TEXT("Conveyor BatchTick"),         STAT_KOConveyorTick,  STATGROUP_Tickables);

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
// 월드 컨텍스트로 서브시스템을 찾아 덤프. 전역 1회 등록이라 PIE 다중 월드에서도 중복 등록 충돌 없음.
static FAutoConsoleCommandWithWorld GKOConveyorDumpCommand(
    TEXT("ko.Conveyor.Dump"),
    TEXT("등록된 모든 컨베이어 벨트 상태를 로그로 덤프."),
    FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
    {
        if (UKOConveyorSubsystem* Subsystem = World ? World->GetSubsystem<UKOConveyorSubsystem>() : nullptr)
        {
            Subsystem->DumpToLog();
        }
    })
);
#endif

UKOConveyorSubsystem* UKOConveyorSubsystem::Get(const UObject* WorldContext)
{
    if (!WorldContext)
    {
        return nullptr;
    }
    const UWorld* World = WorldContext->GetWorld();
    return World ? World->GetSubsystem<UKOConveyorSubsystem>() : nullptr;
}

void UKOConveyorSubsystem::Deinitialize()
{
    Belts.Reset();
    PendingActions.Empty();
    Super::Deinitialize();
}

void UKOConveyorSubsystem::EnqueueAction(TFunction<void()> Action)
{
    if (Action)
    {
        PendingActions.Enqueue(MoveTemp(Action));
    }
}

void UKOConveyorSubsystem::RegisterBelt(AKOConveyorBelt* Belt)
{
    if (!Belt)
    {
        return;
    }
    // 순회 중 변형 방지를 위해 다음 Tick 시작 시점에 반영.
    EnqueueAction([this, WeakBelt = TWeakObjectPtr<AKOConveyorBelt>(Belt)]()
    {
        if (WeakBelt.IsValid())
        {
            Belts.AddUnique(WeakBelt);
        }
    });
}

void UKOConveyorSubsystem::UnregisterBelt(AKOConveyorBelt* Belt)
{
    if (!Belt)
    {
        return;
    }
    EnqueueAction([this, WeakBelt = TWeakObjectPtr<AKOConveyorBelt>(Belt)]()
    {
        Belts.RemoveSingleSwap(WeakBelt);
    });
}

void UKOConveyorSubsystem::ProcessPendingActions()
{
    TFunction<void()> Action;
    while (PendingActions.Dequeue(Action))
    {
        Action();
    }
}

void UKOConveyorSubsystem::Tick(float DeltaTime)
{
    if (DeltaTime <= 0.f)
    {
        return;
    }

    SCOPE_CYCLE_COUNTER(STAT_KOConveyorTick);

    // ① 등록/해제 등 큐잉된 작업을 먼저 반영(이후 순회 중 Belts 변형 없음).
    ProcessPendingActions();

    // ② 모든 벨트 일괄 전진. 무효화된 약참조는 건너뛰고 모아서 정리.
    bool bHasStale = false;
    int32 ItemCount = 0;
    for (const TWeakObjectPtr<AKOConveyorBelt>& WeakBelt : Belts)
    {
        if (AKOConveyorBelt* Belt = WeakBelt.Get())
        {
            Belt->AdvanceBelt(DeltaTime);
            ItemCount += Belt->GetOccupiedSlotCount();
        }
        else
        {
            bHasStale = true;
        }
    }

    SET_DWORD_STAT(STAT_KOConveyorBelts, Belts.Num());
    SET_DWORD_STAT(STAT_KOConveyorItems, ItemCount);

    if (bHasStale)
    {
        Belts.RemoveAll([](const TWeakObjectPtr<AKOConveyorBelt>& WeakBelt)
        {
            return !WeakBelt.IsValid();
        });
    }
}

void UKOConveyorSubsystem::DumpToLog() const
{
    UE_LOG(LogKOConveyor, Log, TEXT("[Conveyor] ===== 벨트 덤프: 등록 %d개 ====="), Belts.Num());

    int32 Index = 0;
    int32 TotalItems = 0;
    for (const TWeakObjectPtr<AKOConveyorBelt>& WeakBelt : Belts)
    {
        const AKOConveyorBelt* Belt = WeakBelt.Get();
        if (!Belt)
        {
            continue;
        }
        TotalItems += Belt->GetOccupiedSlotCount();
        UE_LOG(LogKOConveyor, Log, TEXT("  [%d] %s"), Index, *Belt->DescribeForDebug());
        ++Index;
    }

    UE_LOG(LogKOConveyor, Log, TEXT("[Conveyor] ===== 총 아이템 %d개 ====="), TotalItems);
}

TStatId UKOConveyorSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UKOConveyorSubsystem, STATGROUP_Tickables);
}