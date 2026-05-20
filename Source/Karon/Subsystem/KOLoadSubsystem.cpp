// Copyright Karon Team 5. All Rights Reserved.
#include "Subsystem/KOLoadSubsystem.h"

#include "Data/KODataRegistrySettings.h"
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

template<typename TRow>
void UKOLoadSubsystem::IndexTableRows(
    const TArray<TSoftObjectPtr<UDataTable>>& SoftTables,
    TMap<FGameplayTag, TRow>&                 OutCache,
    TFunctionRef<FGameplayTag(const TRow&)>   GetTag,
    const TCHAR*                              TableKind,
    TArray<UObject*>&                         HardRefs)
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

        // GC가 이 함수 스코프 안에서 Table을 수집하지 못하도록 강한 참조를 유지한다.
        HardRefs.Add(Table);

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

            OutCache.Add(Tag, *Row);
        }

        UE_LOG(LogKOLoad, Log,
            TEXT("UKOLoadSubsystem: Indexed %d %s rows from '%s'."),
            Rows.Num(), TableKind, *Table->GetName());
    }
}

void UKOLoadSubsystem::LoadAll()
{
    // 1. DeveloperSettings 조회 — 엔진 부팅 시 자동 생성/로드되므로 항상 유효 ────
    const UKODataRegistrySettings* Settings = GetDefault<UKODataRegistrySettings>();
    check(Settings);

    // HardRefs: LoadSynchronous로 얻은 UDataTable*을 GC로부터 보호한다.
    // 이 배열이 스코프를 벗어나면 강한 참조가 해제되어 GC 수집이 허용된다.
    TArray<UObject*> HardRefs;

    // 2. 아이템 테이블 로드 및 색인 ────────────────────────────────────────────
    IndexTableRows<FKOItemRow>(
        Settings->ItemTables,
        ItemCache,
        [](const FKOItemRow& Row) { return Row.ItemTag; },
        TEXT("Item"),
        HardRefs);

    // 3. 공장 테이블 로드 및 색인 ──────────────────────────────────────────────
    IndexTableRows<FKOFactoryRow>(
        Settings->FactoryTables,
        FactoryCache,
        [](const FKOFactoryRow& Row) { return Row.FactoryTag; },
        TEXT("Factory"),
        HardRefs);

    // 4. 레시피 테이블 로드 및 색인 ────────────────────────────────────────────
    IndexTableRows<FKORecipeRow>(
        Settings->RecipeTables,
        RecipeCache,
        [](const FKORecipeRow& Row) { return Row.RecipeTag; },
        TEXT("Recipe"),
        HardRefs);

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
    // 1. 약한 참조 캐시 먼저 확인 (중복 디스크 읽기 방지)
    if (const TWeakObjectPtr<UTexture2D>* Cached = ResolvedIcons.Find(ItemTag))
    {
        if (Cached->IsValid())
        {
            return Cached->Get();
        }
        // 약한 참조가 만료된 경우 — 아래에서 재로드
    }

    // 2. 행 조회
    const FKOItemRow* Row = FindItemRow(ItemTag);
    if (!Row)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemIcon: 알 수 없는 ItemTag '%s'."),
            *ItemTag.ToString());
        return nullptr;
    }

    if (Row->Icon.IsNull())
    {
        // 의도적으로 텍스처를 설정하지 않은 경우 — 오류 아님
        return nullptr;
    }

    // 3. 동기 로드
    UTexture2D* Texture = Row->Icon.LoadSynchronous();
    if (!Texture)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemIcon: '%s' 로드 실패 (ItemTag='%s')."),
            *Row->Icon.ToSoftObjectPath().ToString(), *ItemTag.ToString());
        return nullptr;
    }

    // 4. 결과 캐시
    ResolvedIcons.Add(ItemTag, Texture);
    return Texture;
}

UStaticMesh* UKOLoadSubsystem::ResolveItemMesh(FGameplayTag ItemTag) const
{
    // 1. 약한 참조 캐시 확인
    if (const TWeakObjectPtr<UStaticMesh>* Cached = ResolvedMeshes.Find(ItemTag))
    {
        if (Cached->IsValid())
        {
            return Cached->Get();
        }
    }

    // 2. 행 조회
    const FKOItemRow* Row = FindItemRow(ItemTag);
    if (!Row)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemMesh: 알 수 없는 ItemTag '%s'."),
            *ItemTag.ToString());
        return nullptr;
    }

    if (Row->WorldMesh.IsNull())
    {
        return nullptr;
    }

    // 3. 동기 로드
    UStaticMesh* Mesh = Row->WorldMesh.LoadSynchronous();
    if (!Mesh)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemMesh: '%s' 로드 실패 (ItemTag='%s')."),
            *Row->WorldMesh.ToSoftObjectPath().ToString(), *ItemTag.ToString());
        return nullptr;
    }

    // 4. 결과 캐시
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

void UKOLoadSubsystem::GetAllRecipeTags(TArray<FGameplayTag>& Out) const
{
    RecipeCache.GetKeys(Out);
}
