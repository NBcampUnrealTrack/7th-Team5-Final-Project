// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "Containers/Queue.h"
#include "Subsystem/KOItemPortTypes.h"
#include "KOConveyorSubsystem.generated.h"

class AKOConveyorBelt;
class AKOBaseBuilding;

/**
 * 등록된 모든 컨베이어 벨트를 매 프레임 일괄 틱하는 중앙 서브시스템.
 * UKOEnergySubsystem 과 동일한 패턴(WorldSubsystem + FTickableGameObject).
 *
 * 차용 기법(ConveyorPlugin): GT 커맨드 큐.
 * 등록/해제를 즉시 실행하지 않고 큐에 쌓았다가 Tick 시작 시점에 일괄 처리한다.
 * 벨트가 다른 벨트의 push 콜백 도중 파괴/등록되어도 순회 중 배열이 변형되지 않는다.
 * 처리 순서는 항상 "액션(등록/해제) → 그 다음 벨트 순회" 로 고정한다.
 */
UCLASS()
class KARON_API UKOConveyorSubsystem : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

public:
    static UKOConveyorSubsystem* Get(const UObject* WorldContext);

    /** 등록/해제는 GT 커맨드 큐를 거쳐 Tick 안전시점에 반영된다. */
    void RegisterBelt(AKOConveyorBelt* Belt);
    void UnregisterBelt(AKOConveyorBelt* Belt);

    /** 게임스레드 안전시점(다음 Tick 시작)에 실행할 작업을 큐잉. */
    void EnqueueAction(TFunction<void()> Action);

    /** 등록된 모든 벨트 상태를 로그로 덤프(콘솔 ko.Conveyor.Dump 에서 호출). */
    void DumpToLog() const;

    /** (Machine, Kind, PortIndex) 포트가 등록된 벨트 중 하나라도 바인딩 중인가(점유 표시용). */
    bool IsSlotBound(const AKOBaseBuilding* Machine, EKOPortKind Kind, int32 PortIndex) const;

    // FTickableGameObject
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !IsTemplate(); }
    virtual bool IsTickableInEditor() const override { return false; }
    virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }

    // UWorldSubsystem
    virtual void Deinitialize() override;

private:
    void ProcessPendingActions();

    UPROPERTY()
    TArray<TWeakObjectPtr<AKOConveyorBelt>> Belts;

    TQueue<TFunction<void()>> PendingActions;
};