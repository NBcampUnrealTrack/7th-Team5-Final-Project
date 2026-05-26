// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "KOItemTypes.generated.h"

/**
 * 슬롯이 가리키는 데이터 원본 종류.
 * 인벤토리 알고리즘은 이 값을 *식별자의 일부로 분기*하지 않는다 — Add 시점에만 새 슬롯에 기록하고,
 * 외부 소비자(빌드 시스템 / UI)가 슬롯을 어떻게 해석할지 결정할 때 참조한다.
 */
UENUM(BlueprintType)
enum class EKOSlotKind : uint8
{
    Item    UMETA(DisplayName = "Item"),
    Factory UMETA(DisplayName = "Factory"),
};

USTRUCT(BlueprintType)
struct KARON_API FKOItemSlot
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KO|Inventory")
    EKOSlotKind Kind = EKOSlotKind::Item;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KO|Inventory")
    FName ItemId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KO|Inventory")
    int32 Count = 0;

    bool HasItem() const
    {
        return !ItemId.IsNone() && Count > 0;
    }

    bool IsSameItem(const FKOItemSlot& Other) const
    {
        return ItemId == Other.ItemId;
    }
};
