// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "KODataTableTypes.generated.h"

class UTexture2D;
class UStaticMesh;

// AKOFactoryBase: 순환 의존성을 피하기 위해 전방 선언만 사용
class AKOFactoryBase;

/**
 * FKOItemRow
 *
 * 아이템 1종을 정의하는 DataTable 행 구조체.
 * RowName은 가독성을 위해 ItemTag 리프와 맞추는 것을 권장하나,
 * 런타임 TMap 키로는 ItemTag가 사용된다.
 *
 * Icon, WorldMesh는 소프트 레퍼런스로 저장되며 여기서 로드되지 않는다.
 * 실제 로드는 UKOLoadSubsystem::ResolveItemIcon / ResolveItemMesh를 사용한다.
 */
USTRUCT(BlueprintType)
struct KARON_API FKOItemRow : public FTableRowBase
{
    GENERATED_BODY()

    /** 런타임 TMap 키로 사용되는 아이템 고유 태그 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTag ItemTag;

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
 *
 * 공장 건물 1종을 정의하는 DataTable 행 구조체.
 * FactoryClass는 데이터 로드 시점에 Actor BP를 하드 로드하지 않도록
 * 소프트 클래스 포인터로 보관한다.
 */
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

    /** 월드 배치 시 스폰할 Actor 클래스 — 소프트 포인터, 실제 스폰 시점에 해석된다 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    TSoftClassPtr<AKOFactoryBase> FactoryClass;
};

/**
 * FKORecipeRow
 *
 * 제작/생산 레시피 1종을 정의하는 DataTable 행 구조체.
 * Inputs/Outputs는 ItemTag → 수량 매핑이다.
 *
 * 주의: TMap<FGameplayTag, int32>의 UPROPERTY 선언은 UE5.1+ 이상에서만 네이티브 지원된다.
 */
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

    /** 입력 재료: ItemTag → 사이클당 소비 수량 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    TMap<FGameplayTag, int32> Inputs;

    /** 출력 산물: ItemTag → 사이클당 생산 수량 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    TMap<FGameplayTag, int32> Outputs;

    /** 생산 사이클 1회 소요 시간(초) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    float CycleSeconds = 2.f;
};
