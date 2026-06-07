// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Building/KOBaseBuilding.h"
#include "Subsystem/KOItemPortTypes.h"
#include "KOConveyorBelt.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;

/**
 * 벨트 기하 형태. BP/DT 가 지정.
 * 슬롯 큐 시뮬레이션은 형태와 무관하며, 형태는 입구/출구 이웃 셀(InDir/OutDir)만 결정한다.
 */
UENUM()
enum class EKOBeltShape : uint8
{
    Straight,   // 직선: 입구=-Forward 셀, 출구=+Forward 셀.
    Corner      // ㄱ자: 두 열린 다리(+Forward, +Right)를 잇는 90° 회전. bCornerFlip 으로 흐름 반전.
};

/**
 * 슬롯 큐 방식 컨베이어 벨트(직선/코너, M2).
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

    /**
     * 코너 흐름 반전(좌/우 코너 전환)을 직접 지정. 변경 시 입구/출구 방향 즉시 재계산.
     * Straight 형태에선 무시. 보통은 ApplyPlacementFlow 를 쓰고, 이건 강제 지정용.
     */
    void SetCornerFlip(bool bInFlip);

    /**
     * 배치 시스템이 스폰 직후 호출. 코너면 이웃(업/다운스트림)에서 흐름 방향을 자동 추론하고,
     * 모호하거나 이웃이 없으면 bManualFlipFallback(스크롤 회전으로 정한 값)을 사용. 끝에 방향 재계산.
     */
    void ApplyPlacementFlow(bool bManualFlipFallback);

    EKOBeltShape GetShape() const { return Shape; }

    /** 이 벨트의 출구 이웃 셀이 Cell 인가(= 이 벨트가 Cell 로 아이템을 내보내는가). */
    bool OutputsToCell(const FIntPoint& Cell) const { return (MyCell + OutDir) == Cell; }

    /** 이 벨트의 입구 이웃 셀이 Cell 인가(= 이 벨트가 Cell 에서 아이템을 받는가). */
    bool InputsFromCell(const FIntPoint& Cell) const { return (MyCell - InDir) == Cell; }

    /** 현재 아이템이 올라가 있는 슬롯 수(진단용). */
    int32 GetOccupiedSlotCount() const;

    /** 한 줄 디버그 상태 문자열(형태/flip/셀/입출구/적재). 콘솔 덤프용. */
    FString DescribeForDebug() const;

    // IKOItemSource (tail 기준 — 다운스트림 벨트가 pull)
    virtual bool PeekOutputItem(FKOConveyorItem& OutItem) const override;
    virtual bool PopOutputItem(FKOConveyorItem& OutItem) override;

    // IKOItemSink (head 기준 — 외부 push 대비, M1 pull 모델에선 보조)
    virtual bool CanAcceptItem(const FKOConveyorItem& Item) const override;
    virtual bool PushItem(const FKOConveyorItem& Item) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;

    /** 벨트 기하 형태(직선/코너). BP/DT 에서 지정. */
    UPROPERTY(EditAnywhere, Category = "KO|Conveyor")
    EKOBeltShape Shape = EKOBeltShape::Straight;

    /** 슬롯 개수 */
    UPROPERTY(EditAnywhere, Category = "KO|Conveyor", meta = (ClampMin = "1"))
    int32 SlotCount = 8;

    /** 초당 전진 슬롯 수 */
    UPROPERTY(EditAnywhere, Category = "KO|Conveyor", meta = (ClampMin = "0"))
    float SlotsPerSecond = 4.f;

    /** 점유 슬롯에 디버그 스피어 표시. 기본 비주얼은 ItemISM 이므로 기본 off(콘솔 ko.Conveyor.DrawSlots 로 강제 가능). */
    UPROPERTY(EditAnywhere, Category = "KO|Conveyor|Debug")
    bool bDrawSlotsDebug = false;

    // ─── 비주얼(아이템 ISM) ──────────────────────────────────────────────────
    /** 벨트 위 아이템을 그릴 인스턴스 메시 컴포넌트. 슬롯 수만큼 인스턴스를 풀링해 transform 만 갱신. */
    UPROPERTY(VisibleAnywhere, Category = "KO|Conveyor|Visual")
    TObjectPtr<UInstancedStaticMeshComponent> ItemISM;

    /** 아이템 비주얼 메시. 미지정 시 엔진 기본 큐브로 폴백. 디자이너가 BP/DT 에서 지정. */
    UPROPERTY(EditAnywhere, Category = "KO|Conveyor|Visual")
    TObjectPtr<UStaticMesh> ItemMesh;

    /** 슬롯 간격 대비 아이템 비주얼 크기 비율. */
    UPROPERTY(EditAnywhere, Category = "KO|Conveyor|Visual", meta = (ClampMin = "0.05", ClampMax = "1.5"))
    float ItemVisualScale = 0.7f;

    /** 벨트 표면에서 아이템을 띄울 높이(uu). */
    UPROPERTY(EditAnywhere, Category = "KO|Conveyor|Visual")
    float ItemZOffset = 10.f;

private:
    void StepOnce();
    void RecomputePortDirections();
    void DrawSlotsDebug() const;

    /** 입구 모서리→중심→출구 모서리 경로 위 점(T=0~1). 코너면 중심에서 꺾임. 디버그/ISM 공용. */
    FVector ComputeSlotWorldPos(float T) const;

    /** BeginPlay: ItemISM 메시 지정 + 슬롯 수만큼 인스턴스 풀 생성 + 목표 스케일 캐시. */
    void SetupItemVisual();

    /** 매 틱: 점유 슬롯은 경로 위치에 인스턴스 배치, 빈 슬롯은 스케일 0 으로 숨김. */
    void UpdateItemVisual();

    /**
     * 코너 두 다리(+Forward/+Side)의 이웃을 분류해 흐름 방향(flip)을 추론.
     * 한쪽이 명확한 업스트림/다운스트림이면 OutFlip 채우고 true, 모호하면 false.
     */
    bool TryResolveCornerFlipFromNeighbors(bool& OutFlip) const;

    /** 그리드 절대 셀의 점유 액터 조회. */
    AActor* GetActorAtCell(const FIntPoint& Cell) const;

    /** 액터 본체 또는 컴포넌트에서 포트 인터페이스 해결. */
    IKOItemSource* ResolveSource(AActor* Actor) const;
    IKOItemSink*   ResolveSink(AActor* Actor) const;

    /** 슬롯 큐. index0=head(입구), Last=tail(출구). */
    TArray<FKOConveyorItem> Slots;

    /** 슬롯 1칸 전진 누적치(0~1). 비주얼 보간에도 사용. */
    float MoveAccumulator = 0.f;

    /** 코너 흐름 반전 플래그(배치 시 결정). Straight 형태에선 무시. */
    bool bCornerFlip = false;

    // BeginPlay / SetCornerFlip 시점 캐시 (벨트는 이동하지 않음).
    FIntPoint MyCell  = FIntPoint::ZeroValue;
    FIntPoint InDir   = FIntPoint(1, 0);   // 입구 방향(업스트림 = MyCell - InDir)
    FIntPoint OutDir  = FIntPoint(1, 0);   // 출구 방향(다운스트림 = MyCell + OutDir)
    FVector   EntryDirWorld = -FVector::ForwardVector; // 중심 → 입구 이웃 월드 방향(디버그/비주얼)
    FVector   ExitDirWorld  =  FVector::ForwardVector; // 중심 → 출구 이웃 월드 방향(디버그/비주얼)
    float     CellSize = 100.f;

    /** SetupItemVisual 에서 메시 바운드로 산출한 인스턴스 균일 스케일. */
    float     ItemUniformScale = 1.f;
};