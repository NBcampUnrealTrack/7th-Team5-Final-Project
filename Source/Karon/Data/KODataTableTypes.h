// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "KODataTableTypes.generated.h"

class UTexture2D;
class UStaticMesh;

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

USTRUCT(BlueprintType)
struct KARON_API FKOFactoryRow : public FTableRowBase
{
    GENERATED_BODY()

    /** 공장 고유 태그 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    FGameplayTag FactoryTag;

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

    /*/** 월드 배치 시 스폰할 Actor 클래스 — AKOFactoryBase 구현 후 타입 교체 예정 #1#
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    TSoftClassPtr<AKOFactoryBase> FactoryClass;*/
};

USTRUCT(BlueprintType)
struct KARON_API FKORecipeRow : public FTableRowBase
{
    GENERATED_BODY()

    /** 레시피 고유 태그 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    FGameplayTag RecipeTag;

    /**
     * 이 레시피를 처리할 수 있는 공장 태그 집합.
     * 공장의 FactoryTag가 이 컨테이너의 태그 중 하나와 일치하면 처리 가능하다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    FGameplayTagContainer AllowedFactoryTags;

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
