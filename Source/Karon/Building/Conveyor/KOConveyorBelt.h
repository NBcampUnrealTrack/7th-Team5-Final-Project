// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Building/KOBaseBuilding.h"
#include "Subsystem/KOItemPortTypes.h"
#include "KOConveyorBelt.generated.h"

class UInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;
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

    /** 점유 슬롯에 디버그 스피어 표시. 기본 비주얼은 아이템 ISM 이므로 기본 off(콘솔 ko.Conveyor.DrawSlots 로 강제 가능). */
    UPROPERTY(EditAnywhere, Category = "KO|Conveyor|Debug")
    bool bDrawSlotsDebug = false;

    // ─── 비주얼(아이템 ISM) ──────────────────────────────────────────────────
    // 아이템 비주얼은 메시별 ISM 으로 그린다(ISM 1개 = 메시 1종). 각 아이템의 메시는
    // ItemId → DT(FKOItemRow::WorldMesh) 로 해석하며, 미지정 시 ItemMesh(폴백)→엔진 큐브 순.

    /** DT 에 WorldMesh 가 없는 아이템에 쓰는 폴백 메시. 미지정 시 엔진 기본 큐브. 디자이너가 BP 에서 지정. */
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
    void ApplyFlowToMaterial();
    void DrawSlotsDebug() const;

    /** 입구 모서리→중심→출구 모서리 경로 위 점(T=0~1). 코너면 중심에서 꺾임. 디버그/ISM 공용. */
    FVector ComputeSlotWorldPos(float T) const;

    /** BeginPlay: 폴백 메시용 ISM 을 미리 만들어 풀을 준비. 메시별 ISM 은 아이템 등장 시 지연 생성. */
    void SetupItemVisual();

    /** 매 틱: 점유 슬롯은 해당 메시 ISM 에 경로 위치로 배치, 그 외 모든 ISM 의 같은 슬롯은 스케일 0 으로 숨김. */
    void UpdateItemVisual();

    /** ItemId 의 월드 메시 해석(DT → 폴백). 결과를 캐싱. 항상 유효한 메시 반환(엔진 큐브 최종 폴백). */
    UStaticMesh* ResolveItemMesh(FName ItemId);

    /** WorldMesh/ItemMesh 가 모두 없을 때 쓰는 폴백 메시(ItemMesh 우선, 없으면 엔진 큐브). */
    UStaticMesh* GetFallbackMesh();

    /** 주어진 메시용 ISM 을 반환(없으면 런타임 생성·등록 후 슬롯 수만큼 숨김 인스턴스 풀 구성). */
    UInstancedStaticMeshComponent* GetOrCreateISMForMesh(UStaticMesh* Mesh);

    /**
     * 코너 두 다리(+Forward/+Side)의 이웃을 분류해 흐름 방향(flip)을 추론.
     * 한쪽이 명확한 업스트림/다운스트림이면 OutFlip 채우고 true, 모호하면 false.
     */
    bool TryResolveCornerFlipFromNeighbors(bool& OutFlip) const;

    /**
     * 직선 두 축 이웃(+Forward/-Forward)을 분류해 흐름 방향(정/역)을 추론.
     * 한쪽이 명확한 업스트림/다운스트림이면 OutReverse 채우고 true, 모호하면 false.
     */
    bool TryResolveStraightFlowFromNeighbors(bool& OutReverse) const;

    /** 이웃 셀 1칸을 분류: +1=나에게 공급(업스트림), -1=내가 공급(다운스트림), 0=모호/없음. 코너·직선 공용. */
    int32 ClassifyNeighbor(const FIntPoint& MyCellAbs, const FIntPoint& NeighborCell) const;

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

    /** 직선 흐름 반전 플래그(배치 시 이웃 추론으로 결정). Corner 형태에선 무시. false=배치 방향 그대로. */
    bool bStraightReverse = false;

    // BeginPlay / SetCornerFlip 시점 캐시 (벨트는 이동하지 않음).
    FIntPoint MyCell  = FIntPoint::ZeroValue;
    FIntPoint InDir   = FIntPoint(1, 0);   // 입구 방향(업스트림 = MyCell - InDir)
    FIntPoint OutDir  = FIntPoint(1, 0);   // 출구 방향(다운스트림 = MyCell + OutDir)
    FVector   EntryDirWorld = -FVector::ForwardVector; // 중심 → 입구 이웃 월드 방향(디버그/비주얼)
    FVector   ExitDirWorld  =  FVector::ForwardVector; // 중심 → 출구 이웃 월드 방향(디버그/비주얼)
    float     CellSize = 100.f;

    /** 메시별 아이템 ISM. 같은 메시를 쓰는 아이템들은 ISM 하나를 공유(슬롯 인덱스로 인스턴스 식별). */
    UPROPERTY(Transient)
    TMap<TObjectPtr<UStaticMesh>, TObjectPtr<UInstancedStaticMeshComponent>> MeshToISM;

    /** 메시별 인스턴스 균일 스케일(메시 바운드로 산출, 슬롯 간격 비례). */
    TMap<TObjectPtr<UStaticMesh>, float> MeshToScale;

    /** ItemId → 해석된 월드 메시 캐시(DT 조회 1회). */
    TMap<FName, TObjectPtr<UStaticMesh>> ItemMeshCache;

    /** 폴백 큐브 캐시(엔진 BasicShapes 큐브). */
    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> CachedFallbackCube;

    /** 벨트 표면 메시의 DMI 캐시. 흐름 방향 파라미터 설정용. */
    UPROPERTY(Transient)
    TArray<TObjectPtr<UMaterialInstanceDynamic>> BeltDMIs;
};