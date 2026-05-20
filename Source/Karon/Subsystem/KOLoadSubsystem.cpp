// Copyright Karon Team 5. All Rights Reserved.
#include "Subsystem/KOLoadSubsystem.h"

#include "Data/KODataRegistrySettings.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"

// ─── Logging ─────────────────────────────────────────────────────────────────

DEFINE_LOG_CATEGORY_STATIC(LogKOLoad, Log, All);

// ─── Static Accessor ─────────────────────────────────────────────────────────

UKOLoadSubsystem* UKOLoadSubsystem::Get(const UObject* WorldContext)
{
    if (!WorldContext || !GEngine)
    {
        return nullptr;
    }

    const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return nullptr;
    }

    const UGameInstance* GI = World->GetGameInstance();
    return GI ? GI->GetSubsystem<UKOLoadSubsystem>() : nullptr;
}

// ─── USubsystem ──────────────────────────────────────────────────────────────

void UKOLoadSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    LoadAll();
}

void UKOLoadSubsystem::Deinitialize()
{
    // 캐시 먼저 비운 뒤 핀 해제 — 행 포인터 댕글링 방지를 위한 순서.
    ItemCache.Empty();
    FactoryCache.Empty();
    RecipeCache.Empty();
    ResolvedIcons.Empty();
    ResolvedMeshes.Empty();
    LoadedTables.Empty();

    Super::Deinitialize();
}

// ─── Internal Loading ─────────────────────────────────────────────────────────

void UKOLoadSubsystem::IndexItemTables(const TArray<TSoftObjectPtr<UDataTable>>& SoftTables)
{
    for (const TSoftObjectPtr<UDataTable>& SoftTable : SoftTables)
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
        LoadedTables.AddUnique(Table);

        if (Table->GetRowStruct() != FKOItemRow::StaticStruct())
        {
            UE_LOG(LogKOLoad, Error,
                TEXT("UKOLoadSubsystem: ItemTable '%s' RowStruct mismatch — skipping."),
                *Table->GetName());
            continue;
        }

        int32 Count = 0;
        for (const TPair<FName, uint8*>& Pair : Table->GetRowMap())
        {
            const FName ItemId = Pair.Key;
            if (ItemId.IsNone())
            {
                UE_LOG(LogKOLoad, Warning,
                    TEXT("UKOLoadSubsystem: ItemTable '%s' contains a row with empty RowName — skipping."),
                    *Table->GetName());
                continue;
            }

            const FKOItemRow* Row = reinterpret_cast<const FKOItemRow*>(Pair.Value);
            if (!Row)
            {
                continue;
            }

            if (ItemCache.Contains(ItemId))
            {
                UE_LOG(LogKOLoad, Warning,
                    TEXT("UKOLoadSubsystem: Duplicate Item RowName '%s' in table '%s' — existing entry overwritten."),
                    *ItemId.ToString(), *Table->GetName());
            }

            ItemCache.Add(ItemId, Row);
            ++Count;
        }

        UE_LOG(LogKOLoad, Log,
            TEXT("UKOLoadSubsystem: Indexed %d Item rows from '%s'."),
            Count, *Table->GetName());
    }
}

template<typename TRow>
void UKOLoadSubsystem::IndexTableRowsByTag(
    const TArray<TSoftObjectPtr<UDataTable>>& SoftTables,
    TMap<FGameplayTag, const TRow*>&          OutCache,
    TFunctionRef<FGameplayTag(const TRow&)>   GetTag,
    const TCHAR*                              TableKind)
{
    for (const TSoftObjectPtr<UDataTable>& SoftTable : SoftTables)
    {
        if (SoftTable.IsNull())
        {
            UE_LOG(LogKOLoad, Warning,
                TEXT("UKOLoadSubsystem: Null entry in %sTables — skipping."), TableKind);
            continue;
        }

        UDataTable* Table = SoftTable.LoadSynchronous();
        if (!Table)
        {
            UE_LOG(LogKOLoad, Error,
                TEXT("UKOLoadSubsystem: Failed to load %sTable '%s' — skipping."),
                TableKind, *SoftTable.ToSoftObjectPath().ToString());
            continue;
        }
        LoadedTables.AddUnique(Table);

        TArray<TRow*> Rows;
        Table->GetAllRows<TRow>(
            *FString::Printf(TEXT("UKOLoadSubsystem::LoadAll (%ss)"), TableKind), Rows);

        for (const TRow* Row : Rows)
        {
            if (!Row)
            {
                continue;
            }

            const FGameplayTag Tag = GetTag(*Row);
            if (!Tag.IsValid())
            {
                UE_LOG(LogKOLoad, Warning,
                    TEXT("UKOLoadSubsystem: %sTable '%s' contains a row with invalid/empty tag — skipping."),
                    TableKind, *Table->GetName());
                continue;
            }

            if (OutCache.Contains(Tag))
            {
                UE_LOG(LogKOLoad, Warning,
                    TEXT("UKOLoadSubsystem: Duplicate %s tag '%s' detected in table '%s' — existing entry overwritten."),
                    TableKind, *Tag.ToString(), *Table->GetName());
            }

            OutCache.Add(Tag, Row);
        }

        UE_LOG(LogKOLoad, Log,
            TEXT("UKOLoadSubsystem: Indexed %d %s rows from '%s'."),
            Rows.Num(), TableKind, *Table->GetName());
    }
}

void UKOLoadSubsystem::LoadAll()
{
    const UKODataRegistrySettings* Settings = GetDefault<UKODataRegistrySettings>();
    check(Settings);

    IndexItemTables(Settings->ItemTables);

    IndexTableRowsByTag<FKOFactoryRow>(
        Settings->FactoryTables,
        FactoryCache,
        [](const FKOFactoryRow& Row) { return Row.FactoryTag; },
        TEXT("Factory"));

    IndexTableRowsByTag<FKORecipeRow>(
        Settings->RecipeTables,
        RecipeCache,
        [](const FKORecipeRow& Row) { return Row.RecipeTag; },
        TEXT("Recipe"));

    UE_LOG(LogKOLoad, Log,
        TEXT("UKOLoadSubsystem: LoadAll complete. Items=%d, Factories=%d, Recipes=%d."),
        ItemCache.Num(), FactoryCache.Num(), RecipeCache.Num());
}

const FKOItemRow* UKOLoadSubsystem::FindItemRow(FName ItemId) const
{
    const FKOItemRow* const* Found = ItemCache.Find(ItemId);
    return Found ? *Found : nullptr;
}

const FKOFactoryRow* UKOLoadSubsystem::FindFactoryRow(FGameplayTag FactoryTag) const
{
    const FKOFactoryRow* const* Found = FactoryCache.Find(FactoryTag);
    return Found ? *Found : nullptr;
}

const FKORecipeRow* UKOLoadSubsystem::FindRecipeRow(FGameplayTag RecipeTag) const
{
    const FKORecipeRow* const* Found = RecipeCache.Find(RecipeTag);
    return Found ? *Found : nullptr;
}


UTexture2D* UKOLoadSubsystem::ResolveItemIcon(FName ItemId) const
{
    if (const TWeakObjectPtr<UTexture2D>* Cached = ResolvedIcons.Find(ItemId))
    {
        if (Cached->IsValid())
        {
            return Cached->Get();
        }
    }

    const FKOItemRow* Row = FindItemRow(ItemId);
    if (!Row)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemIcon: 알 수 없는 ItemId '%s'."),
            *ItemId.ToString());
        return nullptr;
    }

    if (Row->Icon.IsNull())
    {
        return nullptr;
    }

    UTexture2D* Texture = Row->Icon.LoadSynchronous();
    if (!Texture)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemIcon: '%s' 로드 실패 (ItemId='%s')."),
            *Row->Icon.ToSoftObjectPath().ToString(), *ItemId.ToString());
        return nullptr;
    }

    ResolvedIcons.Add(ItemId, Texture);
    return Texture;
}

UStaticMesh* UKOLoadSubsystem::ResolveItemMesh(FName ItemId) const
{
    if (const TWeakObjectPtr<UStaticMesh>* Cached = ResolvedMeshes.Find(ItemId))
    {
        if (Cached->IsValid())
        {
            return Cached->Get();
        }
    }

    const FKOItemRow* Row = FindItemRow(ItemId);
    if (!Row)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemMesh: 알 수 없는 ItemId '%s'."),
            *ItemId.ToString());
        return nullptr;
    }

    if (Row->WorldMesh.IsNull())
    {
        return nullptr;
    }

    UStaticMesh* Mesh = Row->WorldMesh.LoadSynchronous();
    if (!Mesh)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemMesh: '%s' 로드 실패 (ItemId='%s')."),
            *Row->WorldMesh.ToSoftObjectPath().ToString(), *ItemId.ToString());
        return nullptr;
    }

    ResolvedMeshes.Add(ItemId, Mesh);
    return Mesh;
}

void UKOLoadSubsystem::GetAllItemIds(TArray<FName>& Out) const
{
    ItemCache.GetKeys(Out);
}

void UKOLoadSubsystem::GetAllFactoryTags(TArray<FGameplayTag>& Out) const
{
    FactoryCache.GetKeys(Out);
}

void UKOLoadSubsystem::GetAllRecipeTags(TArray<FGameplayTag>& Out) const
{
    RecipeCache.GetKeys(Out);
}
