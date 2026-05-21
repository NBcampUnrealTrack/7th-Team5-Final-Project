// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data/KODataTableTypes.h"
#include "KOLoadSubsystem.generated.h"

class UTexture2D;
class UStaticMesh;

/**
 * UKOLoadSubsystem
 * 설계에서 최대한 복잡성을 줄였다.
 * 동작 : [GameInstance] Initialize() → LoadAll()
 *
 * [API]
 * FindItemRow(FName) / FindFactoryRow / FindRecipeRow — 키 기반 캐시 조회.
 * ResolveItemIcon / ResolveItemMesh — 소프트 레퍼런스 지연 로드.
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

    const FKOItemRow*    FindItemRow(FName ItemId)              const;
    const FKOFactoryRow* FindFactoryRow(FGameplayTag FactoryTag) const;
    const FKORecipeRow*  FindRecipeRow(FGameplayTag RecipeTag)   const;

    UTexture2D*   ResolveItemIcon(FName ItemId) const;
    UStaticMesh*  ResolveItemMesh(FName ItemId) const;

    void GetAllItemIds(TArray<FName>& Out)             const;
    void GetAllFactoryTags(TArray<FGameplayTag>& Out)  const;
    void GetAllRecipeTags(TArray<FGameplayTag>& Out)   const;

private:
    void LoadAll();
    
    void IndexItemTables(const TArray<TSoftObjectPtr<UDataTable>>& SoftTables);
    
    template<typename TRow>
    void IndexTableRowsByTag(
        const TArray<TSoftObjectPtr<UDataTable>>& SoftTables,
        TMap<FGameplayTag, const TRow*>&          OutCache,
        TFunctionRef<FGameplayTag(const TRow&)>   GetTag,
        const TCHAR*                              TableKind);

    // DataTable 강한 참조를 통해 Row*가 GC에 의해 수거되지 않도록 함
    UPROPERTY()
    TArray<TObjectPtr<UDataTable>> LoadedTables;

    TMap<FName,         const FKOItemRow*>    ItemCache;
    TMap<FGameplayTag,  const FKOFactoryRow*> FactoryCache;
    TMap<FGameplayTag,  const FKORecipeRow*>  RecipeCache;

    // 약한 참조 관련 Mutable 처리
    mutable TMap<FName, TWeakObjectPtr<UTexture2D>>  ResolvedIcons;
    mutable TMap<FName, TWeakObjectPtr<UStaticMesh>> ResolvedMeshes;
};
