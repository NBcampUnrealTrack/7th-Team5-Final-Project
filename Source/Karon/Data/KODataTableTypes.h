// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "KODataTableTypes.generated.h"

class UTexture2D;
class UStaticMesh;
class AKOBaseBuilding;

/**
 * FKOBuildMenuQuery
 * 빌드 메뉴 노출 후보를 조회할 때 사용하는 필터.
 * Category가 None이면 카테고리 제한 없이 전체 대상.
 */
USTRUCT(BlueprintType)
struct KARON_API FKOBuildMenuQuery
{
    GENERATED_BODY()

    /** 빌드 메뉴 카테고리 필터. None이면 전체. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuildMenu")
    FGameplayTag Category;

    /** 플레이어가 보유한 해금 태그. RequiredUnlockTags를 모두 포함해야 노출. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BuildMenu")
    FGameplayTagContainer OwnedUnlocks;
};

USTRUCT(BlueprintType)
struct KARON_API FKOItemRow : public FTableRowBase
{
    GENERATED_BODY()

    /** UI에 표시되는 로컬라이즈드 이름 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    /** 아이템 카테고리 태그 집합 (예: Item.Category.Resource, Item.Category.Weapon) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTagContainer Categories;

    /** 한 인벤토리 슬롯에 최대 누적 가능한 수량 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 MaxStack = 100;

    /** UI 아이콘 텍스처 — 소프트 레퍼런스, UKOLoadSubsystem::ResolveItemIcon()으로 로드 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TSoftObjectPtr<UTexture2D> Icon;

    /** 월드에 배치될 스태틱 메시 — 소프트 레퍼런스, UKOLoadSubsystem::ResolveItemMesh()으로 로드 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TSoftObjectPtr<UStaticMesh> WorldMesh;
};

/**
 * FKOFactoryRow
 * 공장 = 건물. RowName이 곧 FactoryId 이며, Inventory의 ItemId와 동일한 컨벤션이다.
 * 배치 메타(BuildingClass/GridSize/PlacementZOffset)를 함께 보유하여 별도 DataAsset 없이 통합 운영.
 */
USTRUCT(BlueprintType)
struct KARON_API FKOFactoryRow : public FTableRowBase
{
    GENERATED_BODY()

    /** UI에 표시되는 로컬라이즈드 이름 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    FText DisplayName;

    /** 이 공장이 노출하는 아이템 입력 슬롯 수 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    int32 InputSlots = 1;

    /** 이 공장이 노출하는 아이템 출력 슬롯 수 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    int32 OutputSlots = 1;

    /** 기본 생산 사이클 시간(초). 레시피에서 재정의될 수 있다. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    float BaseCycleSeconds = 2.f;

    /** 월드 배치 시 스폰할 건물 액터 클래스 (소프트 레퍼런스) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement")
    TSoftClassPtr<AKOBaseBuilding> BuildingClass;

    /** 그리드 점유 크기 (X, Y 셀 수) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement")
    FIntPoint GridSize = FIntPoint(1, 1);

    /** 바닥보다 살짝 위/아래로 보정할 값 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Placement")
    float PlacementZOffset = 0.f;

    /** 빌드 메뉴에 노출할지 여부. false면 디버그/내부 전용. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BuildMenu")
    bool bShowInBuildMenu = true;

    /** 빌드 메뉴 카테고리 (예: BuildMenu.Category.Production). None이면 무카테고리. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BuildMenu")
    FGameplayTag BuildCategory;

    /** 빌드 메뉴 내 정렬 순서. 작을수록 앞. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BuildMenu")
    int32 SortOrder = 0;

    /** 빌드 메뉴 썸네일 아이콘 (소프트 레퍼런스). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BuildMenu")
    TSoftObjectPtr<UTexture2D> Icon;

    /** 노출되기 위해 플레이어가 보유해야 하는 해금 태그. 비어있으면 항상 해금. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BuildMenu")
    FGameplayTagContainer RequiredUnlockTags;
};

USTRUCT(BlueprintType)
struct KARON_API FKORecipeRow : public FTableRowBase
{
    GENERATED_BODY()

    /**
     * 이 레시피를 처리할 수 있는 공장 RowName 집합.
     * 공장의 RowName이 이 배열에 포함되어 있으면 처리 가능.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    TArray<FName> AllowedFactoryIds;

    /** 입력 재료: ItemId(=Item DataTable의 RowName) → 사이클당 소비 수량 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    TMap<FName, int32> Inputs;

    /** 출력 산물: ItemId(=Item DataTable의 RowName) → 사이클당 생산 수량 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    TMap<FName, int32> Outputs;

    /** 생산 사이클 1회 소요 시간(초) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    float CycleSeconds = 2.f;
};
