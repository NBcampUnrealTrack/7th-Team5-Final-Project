// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/KODataTableTypes.h"
#include "KOLoadSubsystem.generated.h"

class UTexture2D;
class UStaticMesh;
class AKOBaseBuilding;

/**
 * UKOLoadSubsystem
 * 동작 : [GameInstance] Initialize() → LoadAll()
 *
 * [API]
 * FindItemRow / FindFactoryRow / FindRecipeRow — 모두 RowName(FName) 키 기반.
 * ResolveItemIcon / ResolveItemMesh / ResolveBuildingClass — 소프트 레퍼런스 지연 로드.
 */
UCLASS()
class KARON_API UKOLoadSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintPure, Category = "KO|Load", meta = (WorldContext = "WorldContext"))
    static UKOLoadSubsystem* Get(const UObject* WorldContext);

    const FKOItemRow*    FindItemRow(FName ItemId)         const;
    const FKOFactoryRow* FindFactoryRow(FName FactoryId)   const;
    const FKORecipeRow*  FindRecipeRow(FName RecipeId)     const;

    UTexture2D*   ResolveItemIcon(FName ItemId) const;
    UStaticMesh*  ResolveItemMesh(FName ItemId) const;

    /** Factory Row의 BuildingClass 소프트 레퍼런스를 동기 로드 */
    UClass* ResolveBuildingClass(FName FactoryId) const;

    void GetAllItemIds(TArray<FName>& Out)     const;
    void GetAllFactoryIds(TArray<FName>& Out)  const;
    void GetAllRecipeIds(TArray<FName>& Out)   const;

private:
    void LoadAll();

    template<typename TRow>
    void IndexTableRowsByName(
        const TArray<TSoftObjectPtr<UDataTable>>& SoftTables,
        TMap<FName, const TRow*>&                 OutCache,
        const TCHAR*                              TableKind);

    // DataTable 강한 참조를 통해 Row*가 GC에 의해 수거되지 않도록 함
    UPROPERTY()
    TArray<TObjectPtr<UDataTable>> LoadedTables;

    TMap<FName, const FKOItemRow*>    ItemCache;
    TMap<FName, const FKOFactoryRow*> FactoryCache;
    TMap<FName, const FKORecipeRow*>  RecipeCache;

    // 약한 참조 관련 Mutable 처리
    mutable TMap<FName, TWeakObjectPtr<UTexture2D>>  ResolvedIcons;
    mutable TMap<FName, TWeakObjectPtr<UStaticMesh>> ResolvedMeshes;
    mutable TMap<FName, TWeakObjectPtr<UClass>>      ResolvedBuildingClasses;
};
