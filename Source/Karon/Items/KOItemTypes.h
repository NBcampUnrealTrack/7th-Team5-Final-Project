// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "KOItemTypes.generated.h"

/**
 * FKOItemSlot
 *
 * 인벤토리 슬롯 하나의 런타임 값 타입.
 * ItemId(= FKOItemRow의 DataTable RowName) + 수량으로 구성되며,
 * UKOInventoryComponent의 Slots 배열 원소로 사용된다.
 *
 * 이 구조체는 값 타입(Value Object)으로, 동일성 비교는 ItemId 기준이다.
 */
USTRUCT(BlueprintType)
struct KARON_API FKOItemSlot
{
    GENERATED_BODY()
    
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
