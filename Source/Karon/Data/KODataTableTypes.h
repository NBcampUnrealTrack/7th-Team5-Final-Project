// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "KODataTableTypes.generated.h"

// AKOFactoryBase is defined elsewhere; forward declare to avoid circular dependency.
class AKOFactoryBase;

/**
 * FKOItemRow
 *
 * DataTable row describing a single item type.
 * The RowName should match ItemTag's tag leaf for readability, but ItemTag is
 * the authoritative runtime key used in all TMap caches.
 *
 * TSoftObjectPtr fields (Icon, WorldMesh) are NOT loaded here.
 * Call UKOLoadSubsystem::ResolveItemIcon / ResolveItemMesh to trigger a
 * synchronous load on demand.
 */
USTRUCT(BlueprintType)
struct KARON_API FKOItemRow : public FTableRowBase
{
    GENERATED_BODY()

    /** Unique gameplay tag key for this item (used as TMap key at runtime). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTag ItemTag;

    /** Localized display name shown in UI. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FText DisplayName;

    /** Optional category tags (e.g. Item.Category.Resource, Item.Category.Weapon). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    FGameplayTagContainer Categories;

    /** Maximum number of items that can stack in one inventory slot. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    int32 MaxStack = 100;

    /**
     * 2D icon texture — stored as soft reference, loaded on demand via
     * UKOLoadSubsystem::ResolveItemIcon().
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TSoftObjectPtr<UTexture2D> Icon;

    /**
     * Static mesh placed in the world — stored as soft reference, loaded on
     * demand via UKOLoadSubsystem::ResolveItemMesh().
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
    TSoftObjectPtr<UStaticMesh> WorldMesh;
};

/**
 * FKOFactoryRow
 *
 * DataTable row describing a factory building type.
 * FactoryClass is a soft class pointer to avoid hard-loading the Actor BP at
 * data-load time.
 */
USTRUCT(BlueprintType)
struct KARON_API FKOFactoryRow : public FTableRowBase
{
    GENERATED_BODY()

    /** Unique gameplay tag key for this factory type. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    FGameplayTag FactoryTag;

    /** Localized display name shown in UI. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    FText DisplayName;

    /** Number of item input slots this factory exposes. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    int32 InputSlots = 1;

    /** Number of item output slots this factory exposes. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    int32 OutputSlots = 1;

    /** Default cycle duration in seconds (may be overridden by recipe). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    float BaseCycleSeconds = 2.f;

    /**
     * Actor class to spawn when placing this factory in the world.
     * Soft class pointer — resolved only when the factory is actually spawned.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Factory")
    TSoftClassPtr<AKOFactoryBase> FactoryClass;
};

/**
 * FKORecipeRow
 *
 * DataTable row describing a single crafting/production recipe.
 * Inputs and Outputs map ItemTag → quantity required/produced.
 *
 * Note: TMap<FGameplayTag, int32> cannot be a UPROPERTY in all engine
 * versions without plugin support; it is declared as UPROPERTY here because
 * UE5.1+ supports TMap with FGameplayTag keys natively.
 */
USTRUCT(BlueprintType)
struct KARON_API FKORecipeRow : public FTableRowBase
{
    GENERATED_BODY()

    /** Unique gameplay tag key for this recipe. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    FGameplayTag RecipeTag;

    /**
     * Set of factory tags that are allowed to process this recipe.
     * A factory qualifies if its FactoryTag matches any tag in this container.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    FGameplayTagContainer AllowedFactoryTags;

    /** Item inputs: ItemTag → quantity consumed per cycle. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    TMap<FGameplayTag, int32> Inputs;

    /** Item outputs: ItemTag → quantity produced per cycle. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    TMap<FGameplayTag, int32> Outputs;

    /** Duration of one production cycle in seconds. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Recipe")
    float CycleSeconds = 2.f;
};
