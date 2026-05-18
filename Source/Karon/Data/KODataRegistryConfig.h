// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KODataRegistryConfig.generated.h"

class UDataTable;

/**
 * UKODataRegistryConfig
 *
 * 게임 시작 시 로드할 DataTable 목록을 보관하는 PrimaryDataAsset.
 * 인스턴스 경로는 DefaultGame.ini에 설정한다:
 *
 *   [/Script/Karon.KOLoadSubsystem]
 *   DataRegistryConfigPath=/Game/Data/DA_KODataRegistry.DA_KODataRegistry
 *
 * 테이블은 소프트 포인터로 보관하여 에셋 자체를 경량화한다.
 * UKOLoadSubsystem::LoadAll()이 각 테이블을 동기 로드하고 런타임 캐시에 색인한다.
 *
 * PrimaryAssetId: FPrimaryAssetId("KO.Config", <에셋 이름>)
 */
UCLASS(BlueprintType)
class KARON_API UKODataRegistryConfig : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // ─── UPrimaryDataAsset ────────────────────────────────────────────────────
    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(TEXT("KO.Config"), GetFName());
    }

    // ─── Table Lists ──────────────────────────────────────────────────────────

    /** FKOItemRow 행을 가진 DataTable 목록. 여러 테이블 추가 가능하며 전부 하나의 캐시로 병합된다. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> ItemTables;

    /** FKOFactoryRow 행을 가진 DataTable 목록 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> FactoryTables;

    /** FKORecipeRow 행을 가진 DataTable 목록 */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> RecipeTables;
};
