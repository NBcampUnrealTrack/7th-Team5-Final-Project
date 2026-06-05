// Copyright Karon Team 5. All Rights Reserved.

#include "Subsystem/KOConveyorSubsystem.h"

#include "Building/Conveyor/KOConveyorBelt.h"
#include "Engine/World.h"

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

    // ① 등록/해제 등 큐잉된 작업을 먼저 반영(이후 순회 중 Belts 변형 없음).
    ProcessPendingActions();

    // ② 모든 벨트 일괄 전진. 무효화된 약참조는 건너뛰고 모아서 정리.
    bool bHasStale = false;
    for (const TWeakObjectPtr<AKOConveyorBelt>& WeakBelt : Belts)
    {
        if (AKOConveyorBelt* Belt = WeakBelt.Get())
        {
            Belt->AdvanceBelt(DeltaTime);
        }
        else
        {
            bHasStale = true;
        }
    }

    if (bHasStale)
    {
        Belts.RemoveAll([](const TWeakObjectPtr<AKOConveyorBelt>& WeakBelt)
        {
            return !WeakBelt.IsValid();
        });
    }
}

TStatId UKOConveyorSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UKOConveyorSubsystem, STATGROUP_Tickables);
}