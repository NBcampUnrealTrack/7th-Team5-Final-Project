// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "KOItemDefinition.generated.h"

class UTexture2D;
class UStaticMesh;

/**
 * UKOItemDefinition
 *
 * 에디터에서 아이템 1종을 정의하는 PrimaryDataAsset.
 * 런타임 데이터 조회의 주요 경로는 UKOLoadSubsystem (DataTable 캐시)이며,
 * 이 에셋은 에디터 배치 및 Blueprint 참조용으로 병행 사용된다.
 *
 * PrimaryAssetId: ("KO.Item", ItemTag.GetTagName())
 * AssetManager의 PrimaryAssetType에 "KO.Item"을 등록해야 비동기 로드가 지원된다.
 */
UCLASS(BlueprintType, Const)
class KARON_API UKOItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // ─── UPrimaryDataAsset ────────────────────────────────────────────────────

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(TEXT("KO.Item"), ItemTag.GetTagName());
    }

    // ─── Item Identity ────────────────────────────────────────────────────────

    /**
     * 이 아이템의 고유 식별자 태그.
     * TMap 키 및 GMS 메시지 식별에 사용된다.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Item")
    FGameplayTag ItemTag;

    /** UI에 표시되는 로컬라이즈드 이름 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Item")
    FText DisplayName;

    /**
     * 아이템이 속한 카테고리 태그 집합.
     * 예: Item.Category.Resource, Item.Category.Weapon
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Item")
    FGameplayTagContainer Categories;

    /** 한 슬롯에 최대 누적 가능한 수량 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Item", meta = (ClampMin = "1"))
    int32 MaxStack = 100;

    // ─── Visual Assets ────────────────────────────────────────────────────────

    /** UI 인벤토리 슬롯에 표시할 2D 아이콘 (소프트 레퍼런스) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Item|Visual")
    TSoftObjectPtr<UTexture2D> Icon;

    /** 월드에 드롭됐을 때 표시할 스태틱 메시 (소프트 레퍼런스) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Item|Visual")
    TSoftObjectPtr<UStaticMesh> WorldMesh;

    // ─── Gameplay Classification ──────────────────────────────────────────────

    /**
     * 이 아이템이 속하는 코어(공장/제조라인) 식별 태그.
     * 예: Core.Factory.Smelting — 제련 공장에서만 사용되는 아이템임을 표시
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Item|Gameplay")
    FGameplayTag RequiredCoreTag;
};
