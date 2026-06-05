// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Building/KOBaseBuilding.h"
#include "Subsystem/KOItemPortTypes.h"
#include "KOConveyorBelt.generated.h"

/**
 * 슬롯 큐 방식 컨베이어 벨트(직선, M1).
 * - Slots[0] = head(입구), Slots[Last] = tail(출구). NAME_None = 빈칸.
 * - 자체 Tick 없음. UKOConveyorSubsystem 이 매 프레임 AdvanceBelt() 호출.
 * - 그리드 인접 셀로 이웃(머신/벨트)을 해결해 자동 연결.
 * - 전송 규칙: pull 기본 + 머신 sink 에만 tail push (벨트→벨트 이중이동 방지).
 */
UCLASS()
class KARON_API AKOConveyorBelt : public AKOBaseBuilding, public IKOItemSource, public IKOItemSink
{
    GENERATED_BODY()

public:
    AKOConveyorBelt();

    /** 서브시스템이 매 프레임 호출. 누적 후 정수 step 만큼 StepOnce. */
    void AdvanceBelt(float DeltaTime);

    // IKOItemSource (tail 기준 — 다운스트림 벨트가 pull)
    virtual bool PeekOutputItem(FKOConveyorItem& OutItem) const override;
    virtual bool PopOutputItem(FKOConveyorItem& OutItem) override;

    // IKOItemSink (head 기준 — 외부 push 대비, M1 pull 모델에선 보조)
    virtual bool CanAcceptItem(const FKOConveyorItem& Item) const override;
    virtual bool PushItem(const FKOConveyorItem& Item) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

    /** 슬롯 개수 */
    UPROPERTY(EditAnywhere, Category = "KO|Conveyor", meta = (ClampMin = "1"))
    int32 SlotCount = 8;

    /** 초당 전진 슬롯 수 */
    UPROPERTY(EditAnywhere, Category = "KO|Conveyor", meta = (ClampMin = "0"))
    float SlotsPerSecond = 4.f;

    /** 점유 슬롯에 디버그 스피어 표시(M1 비주얼) */
    UPROPERTY(EditAnywhere, Category = "KO|Conveyor|Debug")
    bool bDrawSlotsDebug = true;

private:
    void StepOnce();
    void RecomputePortDirections();
    void DrawSlotsDebug() const;

    /** 그리드 절대 셀의 점유 액터 조회. */
    AActor* GetActorAtCell(const FIntPoint& Cell) const;

    /** 액터 본체 또는 컴포넌트에서 포트 인터페이스 해결. */
    IKOItemSource* ResolveSource(AActor* Actor) const;
    IKOItemSink*   ResolveSink(AActor* Actor) const;

    /** 슬롯 큐. index0=head(입구), Last=tail(출구). */
    TArray<FKOConveyorItem> Slots;

    /** 슬롯 1칸 전진 누적치(0~1). 비주얼 보간에도 사용. */
    float MoveAccumulator = 0.f;

    // BeginPlay 시점 캐시 (벨트는 이동하지 않음).
    FIntPoint MyCell  = FIntPoint::ZeroValue;
    FIntPoint InDir   = FIntPoint(1, 0);   // 입구 방향(업스트림 = MyCell - InDir)
    FIntPoint OutDir  = FIntPoint(1, 0);   // 출구 방향(다운스트림 = MyCell + OutDir)
    FVector   ForwardWorld = FVector::ForwardVector;
    float     CellSize = 100.f;
};