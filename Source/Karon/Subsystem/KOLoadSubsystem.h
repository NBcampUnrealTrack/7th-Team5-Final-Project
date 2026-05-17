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
 *
 * GameInstance-scoped subsystem responsible for loading and caching all
 * DataTable-backed runtime data (items, factories, recipes).
 *
 * Lifecycle
 * ---------
 * Initialize() -> LoadAll()
 *   1. Reads DataRegistryConfigPath from project config
 *      ([/Script/Karon.KOLoadSubsystem] DataRegistryConfigPath=...)
 *   2. Synchronously loads UKODataRegistryConfig
 *   3. Synchronously loads each UDataTable listed in the config
 *   4. Iterates rows and populates the three runtime TMap caches
 *
 * All loading is synchronous (LoadSynchronous) and happens during game
 * instance initialization, before any World is created, so there is no
 * in-game hitch.
 *
 * Query API
 * ---------
 * FindItemRow / FindFactoryRow / FindRecipeRow — O(1) cache lookup by tag.
 * ResolveItemIcon / ResolveItemMesh — lazy synchronous load of soft asset
 *   pointers; results are cached in TWeakObjectPtr maps to avoid redundant
 *   loads while still allowing GC to reclaim unused assets.
 */
UCLASS(Config = Game)
class KARON_API UKOLoadSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ─── USubsystem ──────────────────────────────────────────────────────────
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ─── Row Lookup API ───────────────────────────────────────────────────────

    /**
     * Returns a pointer to the cached FKOItemRow for the given tag, or nullptr
     * if the tag is not registered.
     */
    const FKOItemRow*    FindItemRow(FGameplayTag ItemTag)    const;

    /**
     * Returns a pointer to the cached FKOFactoryRow for the given tag, or
     * nullptr if the tag is not registered.
     */
    const FKOFactoryRow* FindFactoryRow(FGameplayTag FactoryTag) const;

    /**
     * Returns a pointer to the cached FKORecipeRow for the given tag, or
     * nullptr if the tag is not registered.
     */
    const FKORecipeRow*  FindRecipeRow(FGameplayTag RecipeTag)  const;

    // ─── Asset Resolve API ────────────────────────────────────────────────────

    /**
     * Resolves and returns the UTexture2D icon for the given item tag.
     * On first call the soft pointer is loaded synchronously; subsequent calls
     * return the cached result (via TWeakObjectPtr — GC may still collect it).
     * Returns nullptr if the item tag is unknown or the soft path is null.
     */
    UTexture2D*   ResolveItemIcon(FGameplayTag ItemTag) const;

    /**
     * Resolves and returns the UStaticMesh for the given item tag.
     * Same lazy-load + weak-cache pattern as ResolveItemIcon.
     * Returns nullptr if the item tag is unknown or the soft path is null.
     */
    UStaticMesh*  ResolveItemMesh(FGameplayTag ItemTag)  const;

    // ─── Enumeration API ──────────────────────────────────────────────────────

    /** Populates Out with all registered item tags. */
    void GetAllItemTags(TArray<FGameplayTag>& Out)    const;

    /** Populates Out with all registered factory tags. */
    void GetAllFactoryTags(TArray<FGameplayTag>& Out) const;

private:
    // ─── Internal Loading ─────────────────────────────────────────────────────

    /** Drives the full synchronous load pipeline. Called from Initialize(). */
    void LoadAll();

    // ─── Config (populated from DefaultGame.ini via UPROPERTY(Config)) ────────

    /**
     * Soft path to the UKODataRegistryConfig data asset.
     * Set in DefaultGame.ini under [/Script/Karon.KOLoadSubsystem]:
     *   DataRegistryConfigPath=/Game/Data/DA_KODataRegistry.DA_KODataRegistry
     */
    UPROPERTY(Config)
    FSoftObjectPath DataRegistryConfigPath;

    // ─── Runtime Caches ──────────────────────────────────────────────────────
    // These are plain TMap (not UPROPERTY) because:
    //   a) FKOItemRow etc. are FTableRowBase structs, not UObjects.
    //   b) TMap<FGameplayTag, UObject*> caches use TWeakObjectPtr instead.

    TMap<FGameplayTag, FKOItemRow>    ItemCache;
    TMap<FGameplayTag, FKOFactoryRow> FactoryCache;
    TMap<FGameplayTag, FKORecipeRow>  RecipeCache;

    // Weak-ptr caches for resolved assets; mutable to allow const Resolve functions.
    mutable TMap<FGameplayTag, TWeakObjectPtr<UTexture2D>>  ResolvedIcons;
    mutable TMap<FGameplayTag, TWeakObjectPtr<UStaticMesh>> ResolvedMeshes;
};
