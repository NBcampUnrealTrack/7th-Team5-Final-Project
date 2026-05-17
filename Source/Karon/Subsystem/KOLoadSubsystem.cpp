// Copyright Karon Team 5. All Rights Reserved.
#include "Subsystem/KOLoadSubsystem.h"

#include "Data/KODataRegistryConfig.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"

// ─── Logging ─────────────────────────────────────────────────────────────────

DEFINE_LOG_CATEGORY_STATIC(LogKOLoad, Log, All);

// ─── USubsystem ──────────────────────────────────────────────────────────────

void UKOLoadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadAll();
}

void UKOLoadSubsystem::Deinitialize()
{
    ItemCache.Empty();
    FactoryCache.Empty();
    RecipeCache.Empty();
    ResolvedIcons.Empty();
    ResolvedMeshes.Empty();

    Super::Deinitialize();
}

// ─── Internal Loading ─────────────────────────────────────────────────────────

void UKOLoadSubsystem::LoadAll()
{
    // 1. Validate config path ──────────────────────────────────────────────────
    if (!DataRegistryConfigPath.IsValid())
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem: DataRegistryConfigPath is not set. "
                 "Add it to DefaultGame.ini under [/Script/Karon.KOLoadSubsystem]."));
        return;
    }

    // 2. Synchronously load UKODataRegistryConfig ─────────────────────────────
    UKODataRegistryConfig* Config = Cast<UKODataRegistryConfig>(
        DataRegistryConfigPath.TryLoad());

    if (!Config)
    {
        UE_LOG(LogKOLoad, Error,
            TEXT("UKOLoadSubsystem: Failed to load UKODataRegistryConfig at path '%s'. "
                 "Verify the asset exists and the path is correct."),
            *DataRegistryConfigPath.ToString());
        return;
    }

    UE_LOG(LogKOLoad, Log, TEXT("UKOLoadSubsystem: Loaded config '%s'."),
        *Config->GetName());

    // 3. Load & index Item tables ──────────────────────────────────────────────
    for (const TSoftObjectPtr<UDataTable>& SoftTable : Config->ItemTables)
    {
        if (SoftTable.IsNull())
        {
            UE_LOG(LogKOLoad, Warning,
                TEXT("UKOLoadSubsystem: Null entry in ItemTables — skipping."));
            continue;
        }

        UDataTable* Table = SoftTable.LoadSynchronous();
        if (!Table)
        {
            UE_LOG(LogKOLoad, Error,
                TEXT("UKOLoadSubsystem: Failed to load ItemTable '%s' — skipping."),
                *SoftTable.ToSoftObjectPath().ToString());
            continue;
        }

        TArray<FKOItemRow*> Rows;
        Table->GetAllRows<FKOItemRow>(TEXT("UKOLoadSubsystem::LoadAll (Items)"), Rows);

        for (const FKOItemRow* Row : Rows)
        {
            if (!Row || !Row->ItemTag.IsValid())
            {
                UE_LOG(LogKOLoad, Warning,
                    TEXT("UKOLoadSubsystem: ItemTable '%s' contains a row with invalid/empty ItemTag — skipping."),
                    *Table->GetName());
                continue;
            }

            if (ItemCache.Contains(Row->ItemTag))
            {
                UE_LOG(LogKOLoad, Warning,
                    TEXT("UKOLoadSubsystem: Duplicate ItemTag '%s' detected in table '%s' — existing entry overwritten."),
                    *Row->ItemTag.ToString(), *Table->GetName());
            }

            ItemCache.Add(Row->ItemTag, *Row);
        }

        UE_LOG(LogKOLoad, Log,
            TEXT("UKOLoadSubsystem: Indexed %d item rows from '%s'."),
            Rows.Num(), *Table->GetName());
    }

    // 4. Load & index Factory tables ───────────────────────────────────────────
    for (const TSoftObjectPtr<UDataTable>& SoftTable : Config->FactoryTables)
    {
        if (SoftTable.IsNull())
        {
            UE_LOG(LogKOLoad, Warning,
                TEXT("UKOLoadSubsystem: Null entry in FactoryTables — skipping."));
            continue;
        }

        UDataTable* Table = SoftTable.LoadSynchronous();
        if (!Table)
        {
            UE_LOG(LogKOLoad, Error,
                TEXT("UKOLoadSubsystem: Failed to load FactoryTable '%s' — skipping."),
                *SoftTable.ToSoftObjectPath().ToString());
            continue;
        }

        TArray<FKOFactoryRow*> Rows;
        Table->GetAllRows<FKOFactoryRow>(TEXT("UKOLoadSubsystem::LoadAll (Factories)"), Rows);

        for (const FKOFactoryRow* Row : Rows)
        {
            if (!Row || !Row->FactoryTag.IsValid())
            {
                UE_LOG(LogKOLoad, Warning,
                    TEXT("UKOLoadSubsystem: FactoryTable '%s' contains a row with invalid/empty FactoryTag — skipping."),
                    *Table->GetName());
                continue;
            }

            if (FactoryCache.Contains(Row->FactoryTag))
            {
                UE_LOG(LogKOLoad, Warning,
                    TEXT("UKOLoadSubsystem: Duplicate FactoryTag '%s' detected in table '%s' — existing entry overwritten."),
                    *Row->FactoryTag.ToString(), *Table->GetName());
            }

            FactoryCache.Add(Row->FactoryTag, *Row);
        }

        UE_LOG(LogKOLoad, Log,
            TEXT("UKOLoadSubsystem: Indexed %d factory rows from '%s'."),
            Rows.Num(), *Table->GetName());
    }

    // 5. Load & index Recipe tables ────────────────────────────────────────────
    for (const TSoftObjectPtr<UDataTable>& SoftTable : Config->RecipeTables)
    {
        if (SoftTable.IsNull())
        {
            UE_LOG(LogKOLoad, Warning,
                TEXT("UKOLoadSubsystem: Null entry in RecipeTables — skipping."));
            continue;
        }

        UDataTable* Table = SoftTable.LoadSynchronous();
        if (!Table)
        {
            UE_LOG(LogKOLoad, Error,
                TEXT("UKOLoadSubsystem: Failed to load RecipeTable '%s' — skipping."),
                *SoftTable.ToSoftObjectPath().ToString());
            continue;
        }

        TArray<FKORecipeRow*> Rows;
        Table->GetAllRows<FKORecipeRow>(TEXT("UKOLoadSubsystem::LoadAll (Recipes)"), Rows);

        for (const FKORecipeRow* Row : Rows)
        {
            if (!Row || !Row->RecipeTag.IsValid())
            {
                UE_LOG(LogKOLoad, Warning,
                    TEXT("UKOLoadSubsystem: RecipeTable '%s' contains a row with invalid/empty RecipeTag — skipping."),
                    *Table->GetName());
                continue;
            }

            if (RecipeCache.Contains(Row->RecipeTag))
            {
                UE_LOG(LogKOLoad, Warning,
                    TEXT("UKOLoadSubsystem: Duplicate RecipeTag '%s' detected in table '%s' — existing entry overwritten."),
                    *Row->RecipeTag.ToString(), *Table->GetName());
            }

            RecipeCache.Add(Row->RecipeTag, *Row);
        }

        UE_LOG(LogKOLoad, Log,
            TEXT("UKOLoadSubsystem: Indexed %d recipe rows from '%s'."),
            Rows.Num(), *Table->GetName());
    }

    UE_LOG(LogKOLoad, Log,
        TEXT("UKOLoadSubsystem: LoadAll complete. Items=%d, Factories=%d, Recipes=%d."),
        ItemCache.Num(), FactoryCache.Num(), RecipeCache.Num());
}

// ─── Row Lookup API ───────────────────────────────────────────────────────────

const FKOItemRow* UKOLoadSubsystem::FindItemRow(FGameplayTag ItemTag) const
{
    return ItemCache.Find(ItemTag);
}

const FKOFactoryRow* UKOLoadSubsystem::FindFactoryRow(FGameplayTag FactoryTag) const
{
    return FactoryCache.Find(FactoryTag);
}

const FKORecipeRow* UKOLoadSubsystem::FindRecipeRow(FGameplayTag RecipeTag) const
{
    return RecipeCache.Find(RecipeTag);
}

// ─── Asset Resolve API ────────────────────────────────────────────────────────

UTexture2D* UKOLoadSubsystem::ResolveItemIcon(FGameplayTag ItemTag) const
{
    // 1. Check weak-ptr cache first (avoids redundant disk reads).
    if (const TWeakObjectPtr<UTexture2D>* Cached = ResolvedIcons.Find(ItemTag))
    {
        if (Cached->IsValid())
        {
            return Cached->Get();
        }
        // Weak ptr became stale — fall through to re-load.
    }

    // 2. Look up row.
    const FKOItemRow* Row = FindItemRow(ItemTag);
    if (!Row)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemIcon: Unknown ItemTag '%s'."),
            *ItemTag.ToString());
        return nullptr;
    }

    if (Row->Icon.IsNull())
    {
        // Intentionally no texture set — not an error.
        return nullptr;
    }

    // 3. Synchronous load.
    UTexture2D* Texture = Row->Icon.LoadSynchronous();
    if (!Texture)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemIcon: LoadSynchronous failed for '%s' (ItemTag='%s')."),
            *Row->Icon.ToSoftObjectPath().ToString(), *ItemTag.ToString());
        return nullptr;
    }

    // 4. Cache the result.
    ResolvedIcons.Add(ItemTag, Texture);
    return Texture;
}

UStaticMesh* UKOLoadSubsystem::ResolveItemMesh(FGameplayTag ItemTag) const
{
    // 1. Check weak-ptr cache.
    if (const TWeakObjectPtr<UStaticMesh>* Cached = ResolvedMeshes.Find(ItemTag))
    {
        if (Cached->IsValid())
        {
            return Cached->Get();
        }
    }

    // 2. Look up row.
    const FKOItemRow* Row = FindItemRow(ItemTag);
    if (!Row)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemMesh: Unknown ItemTag '%s'."),
            *ItemTag.ToString());
        return nullptr;
    }

    if (Row->WorldMesh.IsNull())
    {
        return nullptr;
    }

    // 3. Synchronous load.
    UStaticMesh* Mesh = Row->WorldMesh.LoadSynchronous();
    if (!Mesh)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemMesh: LoadSynchronous failed for '%s' (ItemTag='%s')."),
            *Row->WorldMesh.ToSoftObjectPath().ToString(), *ItemTag.ToString());
        return nullptr;
    }

    // 4. Cache the result.
    ResolvedMeshes.Add(ItemTag, Mesh);
    return Mesh;
}

// ─── Enumeration API ──────────────────────────────────────────────────────────

void UKOLoadSubsystem::GetAllItemTags(TArray<FGameplayTag>& Out) const
{
    ItemCache.GetKeys(Out);
}

void UKOLoadSubsystem::GetAllFactoryTags(TArray<FGameplayTag>& Out) const
{
    FactoryCache.GetKeys(Out);
}
