// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "KODataRegistryConfig.generated.h"

/**
 * UKODataRegistryConfig
 *
 * PrimaryDataAsset that lists every DataTable the game should load at startup.
 * One instance lives at the path configured in DefaultGame.ini:
 *
 *   [/Script/Karon.KOLoadSubsystem]
 *   DataRegistryConfigPath=/Game/Data/DA_KODataRegistry.DA_KODataRegistry
 *
 * Tables are stored as soft pointers so the asset itself is lightweight.
 * UKOLoadSubsystem::LoadAll() synchronously loads each table and indexes rows
 * into its runtime caches.
 *
 * Asset type/name: "KO.Config" / GetFName(), so the PrimaryAssetId is
 *   FPrimaryAssetId("KO.Config", <asset name>).
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

    /**
     * DataTables whose rows are expected to be FKOItemRow.
     * Add as many tables as needed; all rows will be merged into one cache.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> ItemTables;

    /**
     * DataTables whose rows are expected to be FKOFactoryRow.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> FactoryTables;

    /**
     * DataTables whose rows are expected to be FKORecipeRow.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> RecipeTables;
};
