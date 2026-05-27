// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "KODataTableTypes.generated.h"

class UTexture2D;
class AKOBaseBuilding;

USTRUCT(BlueprintType)
struct KARON_API FKOBuildMenuQuery
{
    GENERATED_BODY()

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

    /**
     * 이 아이템의 고유 식별 태그 (예: "Item.Coal", "Item.BronzePlate").
     * Recipe의 Inputs/Outputs 키로 사용되며, LoadSubsystem에서 ItemId(RowName)와 1:1 매핑된다.
     * 중복 불가 — 같은 태그를 가진 두 ItemRow가 있으면 첫 등록만 유지.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTag ItemTag;

    /** 아이템 카테고리 태그 집합 (예: Item.Category.Resource, Item.Category.Weapon) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTagContainer Categories;

    /** 한 인벤토리 슬롯에 최대 누적 가능한 수량 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 MaxStack = 100;

    /** UI 아이콘 텍스처 — 소프트 레퍼런스, UKOLoadSubsystem::ResolveItemIcon()으로 로드 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TSoftObjectPtr<UTexture2D> Icon;
};

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

    /** 인벤토리 한 슬롯에 누적 가능한 최대 수량. 기본 1 (건물은 통상 비스택). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    int32 MaxStack = 1;

    /**
     * 공장 분류 태그 (예: "Factory.AlloyMaker").
     * Recipe.AllowedFactoryTag와 매칭되어 어떤 레시피가 처리 가능한지 결정한다.
     * 계층 매칭 지원 — Recipe 태그가 부모면 해당 자식 태그를 가진 공장 모두 매칭.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    FGameplayTag FactoryCategoryTag;

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

    /** UI에 표시되는 로컬라이즈드 레시피 이름 (예: "기초 모듈 분해"). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    FText DisplayName;

    /**
     * 이 레시피를 처리할 수 있는 공장 태그.
     * FactoryRow.FactoryCategoryTag와 MatchesTag로 비교 — 계층 지원.
     * 예: AllowedFactoryTag="Factory.AlloyMaker" → AlloyMaker만, "Factory"면 모든 공장.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    FGameplayTag AllowedFactoryTag;

    /** 입력 재료: ItemTag(FKOItemRow::ItemTag) → 사이클당 소비 수량 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    TMap<FGameplayTag, int32> Inputs;

    /** 출력 산물: ItemTag(FKOItemRow::ItemTag) → 사이클당 생산 수량 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    TMap<FGameplayTag, int32> Outputs;

    /** 생산 사이클 1회 소요 시간(초) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    float CycleSeconds = 2.f;

    /**
     * 가공 중(Running) 시간당 에너지 소비량.
     * 0이면 에너지 없이도 가공 가능. EnergySubsystem 공급 비율로 사이클 속도가 감속될 수 있다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    float PowerPerSecond = 0.f;
};
