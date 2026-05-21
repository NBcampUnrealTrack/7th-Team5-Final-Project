// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "KOItemTypes.generated.h"

/**
 * FKOItemSlot
 * 인벤토리 슬롯 하나의 런타임 값 타입.
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
