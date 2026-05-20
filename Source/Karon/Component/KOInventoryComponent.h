// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Items/KOItemTypes.h"
#include "Messaging/KOMessageTypes.h"
#include "KOInventoryComponent.generated.h"

class UGMRouterSubsystem;

/**
 * UKOInventoryComponent
 *
 * 범용 인벤토리 컴포넌트. Slot 기반으로 아이템 식별자(FName)와 수량을 관리한다.
 *
 * 변경 알림: KHS GMS BroadcastMessage(Data_Message_Inventory_Changed) 채널로만 발사한다.
 * 구독은 IKOGMSInterface::Subscribe() 또는 UGMRouterSubsystem::SubscribeMessage() 사용.
 *
 * AcceptedItemsQuery를 설정하면 TryAddItem 시 쿼리를 통과한 아이템만 허용한다.
 */
UCLASS(ClassGroup = "KO|Inventory", meta = (BlueprintSpawnableComponent))
class KARON_API UKOInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UKOInventoryComponent();

    // ─── 설정 ─────────────────────────────────────────────────────────────────

    /** 인벤토리 최대 슬롯 수 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KO|Inventory")
    int32 MaxSlots = 20;

    /**
     * 이 인벤토리가 허용하는 아이템 필터.
     * 빈 쿼리(기본값)면 모든 아이템을 허용한다.
     * 쿼리는 아이템의 FKOItemRow::Categories 태그 컨테이너에 대해 평가된다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Inventory")
    FGameplayTagQuery AcceptedItemsQuery;

    // ─── 아이템 조작 API ──────────────────────────────────────────────────────

    /**
     * 아이템 추가를 시도한다.
     * 기존 슬롯에 스택 가능하면 합산하고, 슬롯이 부족하면 새 슬롯에 분배한다.
     *
     * @param ItemId   추가할 아이템 식별자 (DataTable RowName)
     * @param Count    추가 요청 수량
     * @return         추가하지 못한 잔여 수량 (0이면 전부 추가 성공)
     */
    UFUNCTION(BlueprintCallable, Category = "KO|Inventory")
    virtual int32 TryAddItem(FName ItemId, int32 Count);

    /**
     * 아이템 제거를 시도한다.
     *
     * @param ItemId   제거할 아이템 식별자
     * @param Count    제거 요청 수량
     * @return         요청 수량 전부 제거 성공 여부
     */
    UFUNCTION(BlueprintCallable, Category = "KO|Inventory")
    virtual bool TryRemoveItem(FName ItemId, int32 Count);

    // ─── 조회 API ─────────────────────────────────────────────────────────────

    /**
     * 특정 아이템의 총 보유 수량을 반환한다.
     * 동일 ItemId가 여러 슬롯에 분산된 경우 합산한다.
     */
    UFUNCTION(BlueprintPure, Category = "KO|Inventory")
    int32 GetCountOf(FName ItemId) const;

    /** 슬롯 배열의 const 레퍼런스를 반환한다. */
    UFUNCTION(BlueprintPure, Category = "KO|Inventory")
    const TArray<FKOItemSlot>& GetSlots() const { return Slots; }

    /**
     * 요청한 수량 이상의 아이템을 보유 중인지 확인한다.
     *
     * @param ItemId   확인할 아이템 식별자
     * @param Count    필요한 최소 수량
     */
    UFUNCTION(BlueprintPure, Category = "KO|Inventory")
    bool HasEnoughItems(FName ItemId, int32 Count) const;

protected:
    /** 슬롯 배열. UPROPERTY로 GC에 등록된다. */
    UPROPERTY()
    TArray<FKOItemSlot> Slots;

    /**
     * 슬롯 변경 후 반드시 호출한다.
     * KHS GMS로 FKOInventoryChangedMessage를 Data_Message_Inventory_Changed 채널에 브로드캐스트한다.
     *
     * @param ItemId        변경된 아이템 식별자
     * @param PreviousCount 변경 전 수량
     * @param NewCount      변경 후 수량
     */
    void NotifyChanged(FName ItemId, int32 PreviousCount, int32 NewCount);

    /**
     * AcceptedItemsQuery 기반 아이템 허용 여부 검사.
     * UKOLoadSubsystem에서 카테고리 태그를 조회한 뒤 쿼리를 평가한다.
     * 쿼리가 비어 있으면 항상 true를 반환한다.
     */
    virtual bool IsItemAccepted(FName ItemId) const;

    /** 아이템의 MaxStack 값을 UKOLoadSubsystem에서 조회한다. 없으면 100 반환 */
    int32 GetMaxStackForItem(FName ItemId) const;
};
