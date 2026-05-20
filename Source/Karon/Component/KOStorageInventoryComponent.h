// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Component/KOInventoryComponent.h"
#include "GameplayTagContainer.h"
#include "KOStorageInventoryComponent.generated.h"

/**
 * UKOStorageInventoryComponent
 *
 * 창고(Storage) 전용 인벤토리 컴포넌트.
 * UKOInventoryComponent를 확장하여:
 *   - AllowedCategories 태그 컨테이너로 아이템 종류를 제한한다.
 *   - ExtraSlots만큼 추가 슬롯을 제공한다.
 *   - bAutoSortOnChange가 true이면 변경 시 자동으로 슬롯을 정렬한다.
 *
 * BeginPlay에서 MaxSlots += ExtraSlots가 처리된다.
 */
UCLASS(ClassGroup = "KO|Inventory", meta = (BlueprintSpawnableComponent))
class KARON_API UKOStorageInventoryComponent : public UKOInventoryComponent
{
    GENERATED_BODY()

public:
    UKOStorageInventoryComponent();

    // ─── 창고 전용 설정 ───────────────────────────────────────────────────────

    /**
     * 이 창고에 보관 가능한 아이템 카테고리 태그.
     * 비어 있으면 모든 카테고리를 허용한다.
     * 평가는 FKOItemRow::Categories 태그 컨테이너와의 HasAny 매칭으로 수행된다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KO|Storage")
    FGameplayTagContainer AllowedCategories;

    /** 기본 MaxSlots에 추가되는 슬롯 수. BeginPlay에서 합산된다. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KO|Storage", meta = (ClampMin = "0"))
    int32 ExtraSlots = 40;

    /**
     * true이면 아이템 변경 시 슬롯 배열을 ItemId 기준으로 자동 정렬한다.
     * 정렬 비용은 O(N log N)이므로 슬롯이 매우 많을 때 주의한다.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KO|Storage")
    bool bAutoSortOnChange = true;

    // ─── 공개 API ─────────────────────────────────────────────────────────────

    /**
     * 아이템이 이 창고의 AllowedCategories 조건을 통과하는지 확인한다.
     * AllowedCategories가 비어 있으면 항상 true를 반환한다.
     *
     * @param ItemId 검사할 아이템 식별자
     */
    UFUNCTION(BlueprintPure, Category = "KO|Storage")
    bool CanAcceptItem(FName ItemId) const;

    // ─── TryAddItem 재정의 ────────────────────────────────────────────────────

    /** CanAcceptItem 검사 후 부모의 TryAddItem을 호출한다. */
    virtual int32 TryAddItem(FName ItemId, int32 Count) override;

protected:
    // ─── UActorComponent ──────────────────────────────────────────────────────

    virtual void BeginPlay() override;

    // ─── IsItemAccepted 재정의 ────────────────────────────────────────────────

    /**
     * 부모의 AcceptedItemsQuery 검사에 더해 AllowedCategories도 검사한다.
     */
    virtual bool IsItemAccepted(FName ItemId) const override;

private:
    /** Slots를 ItemId 기준으로 오름차순 정렬한다. */
    void SortSlots();
};
