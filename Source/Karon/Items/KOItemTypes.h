// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KOItemTypes.generated.h"

/**
 * FKOItemSlot
 *
 * 인벤토리 슬롯 하나의 런타임 값 타입.
 * ItemTag + 수량으로 구성되며, UKOInventoryComponent의 Slots 배열 원소로 사용된다.
 *
 * 이 구조체는 값 타입(Value Object)으로, 동일성 비교는 ItemTag 기준이다.
 */
USTRUCT(BlueprintType)
struct KARON_API FKOItemSlot
{
    GENERATED_BODY()

    /** 슬롯에 들어있는 아이템 식별 태그. 빈 슬롯이면 Invalid 태그 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KO|Inventory")
    FGameplayTag ItemTag;

    /** 현재 누적 수량. 0이면 빈 슬롯으로 간주 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KO|Inventory")
    int32 Count = 0;

    // ─── 유틸리티 ─────────────────────────────────────────────────────────────

    /** 슬롯에 유효한 아이템이 있으면 true (Tag 유효 & Count > 0) */
    bool IsValid() const
    {
        return ItemTag.IsValid() && Count > 0;
    }

    /**
     * 두 슬롯이 같은 아이템 종류인지 반환.
     * Count는 비교하지 않는다.
     *
     * @param Other 비교 대상 슬롯
     */
    bool IsSameItem(const FKOItemSlot& Other) const
    {
        return ItemTag == Other.ItemTag;
    }
};
