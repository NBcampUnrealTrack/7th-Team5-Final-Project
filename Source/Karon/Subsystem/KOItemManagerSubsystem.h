// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KOItemManagerSubsystem.generated.h"

class UKOInventoryComponent;

/**
 * UKOItemManagerSubsystem
 *
 * GameInstance 수명 아이템 관리 서브시스템.
 * UKOLoadSubsystem(DataTable 캐시)에 의존하여 아이템 정의를 조회하고,
 * 인벤토리 간 이전(Transfer), 스택 분리(SplitStack), 스택 병합(MergeAllStacks) 유스케이스를 제공한다.
 *
 * 아이템 식별자는 FName(= Item DataTable RowName) 단일이다.
 *
 * 의존성:
 *   - UKOLoadSubsystem (Initialize에서 InitializeDependency로 선행 초기화 보장)
 */
UCLASS()
class KARON_API UKOItemManagerSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ─── USubsystem ──────────────────────────────────────────────────────────
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /**
     * WorldContext로부터 이 서브시스템 인스턴스를 안전하게 획득한다.
     * @return 찾으면 인스턴스, 못 찾으면 nullptr
     */
    UFUNCTION(BlueprintPure, Category = "KO|Item", meta = (WorldContext = "WorldContext"))
    static UKOItemManagerSubsystem* Get(const UObject* WorldContext);

    // ─── 아이템 최대 스택 조회 ────────────────────────────────────────────────

    /**
     * ItemId에 해당하는 MaxStack 값을 반환한다.
     * 조회 순서: UKOLoadSubsystem::FindItemRow → 기본값 100.
     *
     * @param ItemId 조회할 아이템 식별자
     * @return       최대 스택 수량
     */
    UFUNCTION(BlueprintPure, Category = "KO|Item")
    int32 GetMaxStack(FName ItemId) const;

    // ─── 인벤토리 유스케이스 ──────────────────────────────────────────────────

    /**
     * From 인벤토리에서 To 인벤토리로 아이템을 이전한다.
     * To에 공간이 없거나 From에 수량이 부족하면 이전 가능한 만큼만 이전한다.
     *
     * @param From           출발 인벤토리
     * @param To             도착 인벤토리
     * @param ItemId         이전할 아이템 식별자
     * @param RequestedCount 요청 이전 수량
     * @return               실제로 이전된 수량
     */
    UFUNCTION(BlueprintCallable, Category = "KO|Item")
    int32 TransferItems(
        UKOInventoryComponent* From,
        UKOInventoryComponent* To,
        FName                  ItemId,
        int32                  RequestedCount
    );

    /**
     * 지정 슬롯의 스택을 SplitCount만큼 분리하여 새 슬롯에 배치한다.
     * 분리 후 원래 슬롯의 수량이 0이 되면 해당 슬롯은 제거되지 않고
     * 인벤토리 컴포넌트 내부 로직에 위임한다.
     *
     * @param Inventory   대상 인벤토리
     * @param SlotIndex   분리할 슬롯 인덱스
     * @param SplitCount  분리할 수량 (>= 1, < 슬롯 수량)
     * @return            분리 성공 여부
     */
    UFUNCTION(BlueprintCallable, Category = "KO|Item")
    bool SplitStack(UKOInventoryComponent* Inventory, int32 SlotIndex, int32 SplitCount);

    /**
     * 인벤토리 내 동일 ItemId의 모든 슬롯을 병합하여 스택 수를 최소화한다.
     * 각 아이템마다 MaxStack 단위로 슬롯을 재구성한다.
     *
     * @param Inventory 병합 대상 인벤토리
     */
    UFUNCTION(BlueprintCallable, Category = "KO|Item")
    void MergeAllStacks(UKOInventoryComponent* Inventory);
};
