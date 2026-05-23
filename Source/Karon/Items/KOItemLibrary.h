// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "Data/KODataTableTypes.h"
#include "KOItemLibrary.generated.h"

class UTexture2D;
class UStaticMesh;
class UKOLoadSubsystem;

/**
 * UKOItemLibrary
 * 아이템 데이터 조회용 정적 유틸리티.
 * 사용 예시 : FText Name = UKOItemLibrary::GetItemDisplayName(this, Slot.ItemId);
 */
UCLASS()
class KARON_API UKOItemLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    static bool GetItemRow(const UObject* WorldContext, FName ItemId, FKOItemRow& OutRow);

    static FText GetItemDisplayName(const UObject* WorldContext, FName ItemId);

    static int32 GetItemMaxStack(const UObject* WorldContext, FName ItemId);

    static FGameplayTagContainer GetItemCategories(const UObject* WorldContext, FName ItemId);

    static UTexture2D* GetItemIcon(const UObject* WorldContext, FName ItemId);

    static UStaticMesh* GetItemMesh(const UObject* WorldContext, FName ItemId);

private:
    static const UKOLoadSubsystem* GetLoadSubsystem(const UObject* WorldContext);
};
