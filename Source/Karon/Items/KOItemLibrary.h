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
 *
 * 아이템 데이터 조회용 BlueprintFunctionLibrary.
 * 모든 함수는 ItemId(=Item DataTable RowName)를 키로 UKOLoadSubsystem 캐시를 조회한다.
 *
 * 사용 예 (C++):
 *   FText Name = UKOItemLibrary::GetItemDisplayName(this, Slot.ItemId);
 *
 * 사용 예 (BP):
 *   Get Item Display Name (World Context: Self, Item Id: ...)
 */
UCLASS()
class KARON_API UKOItemLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * ItemId에 해당하는 행 데이터를 OutRow에 채워 반환한다.
     * @return 행을 찾으면 true. 못 찾으면 false (OutRow는 미변경)
     */
    UFUNCTION(BlueprintPure, Category = "KO|Item", meta = (WorldContext = "WorldContext"))
    static bool GetItemRow(const UObject* WorldContext, FName ItemId, FKOItemRow& OutRow);

    /** ItemId → DisplayName. 행이 없으면 빈 FText. */
    UFUNCTION(BlueprintPure, Category = "KO|Item", meta = (WorldContext = "WorldContext"))
    static FText GetItemDisplayName(const UObject* WorldContext, FName ItemId);

    /** ItemId → MaxStack. 행이 없으면 기본값 100. */
    UFUNCTION(BlueprintPure, Category = "KO|Item", meta = (WorldContext = "WorldContext"))
    static int32 GetItemMaxStack(const UObject* WorldContext, FName ItemId);

    /** ItemId → Categories. 행이 없으면 빈 컨테이너. */
    UFUNCTION(BlueprintPure, Category = "KO|Item", meta = (WorldContext = "WorldContext"))
    static FGameplayTagContainer GetItemCategories(const UObject* WorldContext, FName ItemId);

    /** ItemId → Icon. 소프트 레퍼런스를 동기 로드하며 결과는 UKOLoadSubsystem에서 캐시된다. */
    UFUNCTION(BlueprintPure, Category = "KO|Item", meta = (WorldContext = "WorldContext"))
    static UTexture2D* GetItemIcon(const UObject* WorldContext, FName ItemId);

    /** ItemId → WorldMesh. 소프트 레퍼런스를 동기 로드하며 결과는 UKOLoadSubsystem에서 캐시된다. */
    UFUNCTION(BlueprintPure, Category = "KO|Item", meta = (WorldContext = "WorldContext"))
    static UStaticMesh* GetItemMesh(const UObject* WorldContext, FName ItemId);

private:
    /** WorldContext로부터 UKOLoadSubsystem을 안전하게 획득한다. 못 찾으면 nullptr. */
    static const UKOLoadSubsystem* GetLoadSubsystem(const UObject* WorldContext);
};
