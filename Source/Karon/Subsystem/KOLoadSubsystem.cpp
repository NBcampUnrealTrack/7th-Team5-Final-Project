// Copyright Karon Team 5. All Rights Reserved.
#include "Subsystem/KOLoadSubsystem.h"

#include "Building/KOBaseBuilding.h"
#include "Data/KODataRegistrySettings.h"
#include "Data/Equipment/KOWeaponDefinition.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"


DEFINE_LOG_CATEGORY_STATIC(LogKOLoad, Log, All);

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
    EquipmentCache.Empty();
    SkillCache.Empty();
    SkillExecutionCache.Empty();
    
    ItemTagToId.Empty();
    EquipmentTagToId.Empty();
    
    ResolvedIcons.Empty();
    ResolvedBuildingClasses.Empty();
    ResolvedFactoryIcons.Empty();
    ResolvedSkillIcons.Empty();
    LoadedTables.Empty();
    ResolvedItemMeshes.Empty();

    Super::Deinitialize();
}

template<typename TRow>
void UKOLoadSubsystem::IndexTableRowsByName(
    const TArray<TSoftObjectPtr<UDataTable>>& SoftTables,
    TMap<FName, const TRow*>&                 OutCache,
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

        if (Table->GetRowStruct() != TRow::StaticStruct())
        {
            UE_LOG(LogKOLoad, Error,
                TEXT("UKOLoadSubsystem: %sTable '%s' RowStruct mismatch — skipping."),
                TableKind, *Table->GetName());
            continue;
        }

        int32 Count = 0;
        for (const TPair<FName, uint8*>& Pair : Table->GetRowMap())
        {
            const FName RowName = Pair.Key;
            if (RowName.IsNone())
            {
                UE_LOG(LogKOLoad, Warning,
                    TEXT("UKOLoadSubsystem: %sTable '%s' contains a row with empty RowName — skipping."),
                    TableKind, *Table->GetName());
                continue;
            }

            const TRow* Row = reinterpret_cast<const TRow*>(Pair.Value);
            if (!Row)
            {
                continue;
            }

            if (OutCache.Contains(RowName))
            {
                UE_LOG(LogKOLoad, Warning,
                    TEXT("UKOLoadSubsystem: Duplicate %s RowName '%s' in table '%s' — existing entry overwritten."),
                    TableKind, *RowName.ToString(), *Table->GetName());
            }

            OutCache.Add(RowName, Row);
            ++Count;
        }

        UE_LOG(LogKOLoad, Log,
            TEXT("UKOLoadSubsystem: Indexed %d %s rows from '%s'."),
            Count, TableKind, *Table->GetName());
    }
}

void UKOLoadSubsystem::LoadAll()
{
    const UKODataRegistrySettings* Settings = GetDefault<UKODataRegistrySettings>();
    check(Settings);

    IndexTableRowsByName<FKOItemRow>      (Settings->ItemTables,      ItemCache,      TEXT("Item"));
    IndexTableRowsByName<FKOFactoryRow>   (Settings->FactoryTables,   FactoryCache,   TEXT("Factory"));
    IndexTableRowsByName<FKORecipeRow>    (Settings->RecipeTables,    RecipeCache,    TEXT("Recipe"));
    IndexTableRowsByName<FKOEquipmentRow> (Settings->EquipmentTables, EquipmentCache, TEXT("Equipment"));
    IndexTableRowsByName<FKOSkillRow>          (Settings->SkillTables,          SkillCache,          TEXT("Skill"));
    IndexTableRowsByName<FKOSkillExecutionRow> (Settings->SkillExecutionTables, SkillExecutionCache, TEXT("SkillExecution"));
    
    // ItemTag → ItemId 역인덱스 빌드
    ItemTagToId.Reset();
    for (const TPair<FName, const FKOItemRow*>& Pair : ItemCache)
    {
        const FKOItemRow* Row = Pair.Value;
        if (!Row || !Row->ItemTag.IsValid())
        {
            continue;
        }

        if (ItemTagToId.Contains(Row->ItemTag))
        {
            UE_LOG(LogKOLoad, Warning,
                TEXT("UKOLoadSubsystem: 중복 ItemTag '%s' (기존='%s', 신규='%s' — 무시)."),
                *Row->ItemTag.ToString(),
                *ItemTagToId[Row->ItemTag].ToString(),
                *Pair.Key.ToString());
            continue;
        }

        ItemTagToId.Add(Row->ItemTag, Pair.Key);
    }
    
    // Equipment ItemTag → EquipmentId 역인덱스 빌드
    EquipmentTagToId.Reset();
    for (const TPair<FName, const FKOEquipmentRow*>& Pair : EquipmentCache)
    {
        const FKOEquipmentRow* Row = Pair.Value;
        if (!Row || !Row->ItemTag.IsValid())
        {
            continue;
        }

        if (EquipmentTagToId.Contains(Row->ItemTag))
        {
            UE_LOG(LogKOLoad, Warning,
                TEXT("UKOLoadSubsystem: 중복 Equipment ItemTag '%s' (기존='%s', 신규='%s' — 무시)."),
                *Row->ItemTag.ToString(),
                *EquipmentTagToId[Row->ItemTag].ToString(),
                *Pair.Key.ToString());
            continue;
        }

        EquipmentTagToId.Add(Row->ItemTag, Pair.Key);
    }

    UE_LOG(LogKOLoad, Log,
        TEXT("UKOLoadSubsystem: LoadAll complete. Items=%d, Factories=%d, Recipes=%d, ItemTags=%d."),
        ItemCache.Num(), FactoryCache.Num(), RecipeCache.Num(), ItemTagToId.Num());
}

FName UKOLoadSubsystem::FindItemIdByTag(FGameplayTag ItemTag) const
{
    if (!ItemTag.IsValid())
    {
        return NAME_None;
    }
    const FName* Found = ItemTagToId.Find(ItemTag);
    return Found ? *Found : NAME_None;
}

bool UKOLoadSubsystem::IsEquipmentItem(FGameplayTag ItemTag) const
{
    return FindEquipmentRowByItemTag(ItemTag) != nullptr;
}

const FKOItemRow* UKOLoadSubsystem::FindItemRow(FName ItemId) const
{
    const FKOItemRow* const* Found = ItemCache.Find(ItemId);
    return Found ? *Found : nullptr;
}

const FKOFactoryRow* UKOLoadSubsystem::FindFactoryRow(FName FactoryId) const
{
    const FKOFactoryRow* const* Found = FactoryCache.Find(FactoryId);
    return Found ? *Found : nullptr;
}

const FKORecipeRow* UKOLoadSubsystem::FindRecipeRow(FName RecipeId) const
{
    const FKORecipeRow* const* Found = RecipeCache.Find(RecipeId);
    return Found ? *Found : nullptr;
}

const FKOEquipmentRow* UKOLoadSubsystem::FindEquipmentRow(FName EquipmentId) const
{
    const FKOEquipmentRow* const* Found = EquipmentCache.Find(EquipmentId);
    return Found ? *Found : nullptr;
}

const FKOEquipmentRow* UKOLoadSubsystem::FindEquipmentRowByItemTag(FGameplayTag ItemTag) const
{
    if (!ItemTag.IsValid())
    {
        return nullptr;
    }

    const FName* EquipmentId = EquipmentTagToId.Find(ItemTag);
    if (!EquipmentId)
    {
        return nullptr;
    }

    return FindEquipmentRow(*EquipmentId);
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
    if (const TWeakObjectPtr<UStaticMesh>* Cached = ResolvedItemMeshes.Find(ItemId))
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
        return nullptr; // 메시 미지정 — 호출 측에서 폴백 처리.
    }

    UStaticMesh* Mesh = Row->WorldMesh.LoadSynchronous();
    if (!Mesh)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveItemMesh: '%s' 로드 실패 (ItemId='%s')."),
            *Row->WorldMesh.ToSoftObjectPath().ToString(), *ItemId.ToString());
        return nullptr;
    }

    ResolvedItemMeshes.Add(ItemId, Mesh);
    return Mesh;
}

UClass* UKOLoadSubsystem::ResolveBuildingClass(FName FactoryId) const
{
    if (const TWeakObjectPtr<UClass>* Cached = ResolvedBuildingClasses.Find(FactoryId))
    {
        if (Cached->IsValid())
        {
            return Cached->Get();
        }
    }

    const FKOFactoryRow* Row = FindFactoryRow(FactoryId);
    if (!Row)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveBuildingClass: 알 수 없는 FactoryId '%s'."),
            *FactoryId.ToString());
        return nullptr;
    }

    if (Row->BuildingClass.IsNull())
    {
        return nullptr;
    }

    UClass* Loaded = Row->BuildingClass.LoadSynchronous();
    if (!Loaded)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveBuildingClass: '%s' 로드 실패 (FactoryId='%s')."),
            *Row->BuildingClass.ToSoftObjectPath().ToString(), *FactoryId.ToString());
        return nullptr;
    }

    ResolvedBuildingClasses.Add(FactoryId, Loaded);
    return Loaded;
}

UKOWeaponDefinition* UKOLoadSubsystem::ResolveWeaponDefinitionByItemId(FName ItemId) const
{
    if (ItemId.IsNone())
    {
        return nullptr;
    }

    const FKOItemRow* ItemRow = FindItemRow(ItemId);
    if (!ItemRow)
    {
        UE_LOG(LogKOLoad, Warning, TEXT("[LoadSubsystem] ItemRow 없음: %s"), *ItemId.ToString());
        return nullptr;
    }

    const FKOEquipmentRow* EquipmentRow =
        FindEquipmentRowByItemTag(ItemRow->ItemTag);

    if (!EquipmentRow)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("[LoadSubsystem] EquipmentRow 없음: ItemId=%s, ItemTag=%s"),
            *ItemId.ToString(),
            *ItemRow->ItemTag.ToString()
        );
        return nullptr;
    }

    if (EquipmentRow->SlotType != EKOEquipmentSlotType::Weapon)
    {
        return nullptr;
    }

    if (EquipmentRow->WeaponDefinition.IsNull())
    {
        UE_LOG(LogKOLoad, Warning, TEXT("[LoadSubsystem] WeaponDefinition 미설정: %s"), *ItemId.ToString());
        return nullptr;
    }

    return EquipmentRow->WeaponDefinition.LoadSynchronous();
}

void UKOLoadSubsystem::GetAllItemIds(TArray<FName>& Out) const
{
    ItemCache.GetKeys(Out);
}

void UKOLoadSubsystem::GetAllFactoryIds(TArray<FName>& Out) const
{
    FactoryCache.GetKeys(Out);
}

void UKOLoadSubsystem::GetAllRecipeIds(TArray<FName>& Out) const
{
    RecipeCache.GetKeys(Out);
}

void UKOLoadSubsystem::GetAllEquipmentIds(TArray<FName>& Out) const
{
    EquipmentCache.GetKeys(Out);
}

void UKOLoadSubsystem::GetCraftableEquipmentIds(TArray<FName>& Out) const
{
    Out.Reset();

    struct FCandidate
    {
        FName Id;
    };

    TArray<FCandidate> Candidates;
    Candidates.Reserve(EquipmentCache.Num());

    for (const TPair<FName, const FKOEquipmentRow*>& Pair : EquipmentCache)
    {
        const FKOEquipmentRow* Row = Pair.Value;
        if (!Row)
        {
            continue;
        }

        if (!Row->bCraftable)
        {
            continue;
        }

        Candidates.Add({ Pair.Key });
    }

    Candidates.Sort([](const FCandidate& A, const FCandidate& B)
    {
        return A.Id.LexicalLess(B.Id);
    });

    Out.Reserve(Candidates.Num());
    for (const FCandidate& Candidate : Candidates)
    {
        Out.Add(Candidate.Id);
    }
}

void UKOLoadSubsystem::GetBuildableFactoryIds(const FKOBuildMenuQuery& Query, TArray<FName>& Out) const
{
    Out.Reset();

    struct FCandidate
    {
        FName Id;
        int32 SortOrder;
    };
    TArray<FCandidate> Candidates;
    Candidates.Reserve(FactoryCache.Num());

    for (const TPair<FName, const FKOFactoryRow*>& Pair : FactoryCache)
    {
        const FKOFactoryRow* Row = Pair.Value;
        if (!Row || !Row->bShowInBuildMenu)
        {
            continue;
        }

        if (!Row->RequiredUnlockTags.IsEmpty() &&
            !Query.OwnedUnlocks.HasAll(Row->RequiredUnlockTags))
        {
            continue;
        }

        Candidates.Add({ Pair.Key, Row->SortOrder });
    }

    Candidates.Sort([](const FCandidate& A, const FCandidate& B)
    {
        if (A.SortOrder != B.SortOrder)
        {
            return A.SortOrder < B.SortOrder;
        }
        return A.Id.LexicalLess(B.Id);
    });

    Out.Reserve(Candidates.Num());
    for (const FCandidate& C : Candidates)
    {
        Out.Add(C.Id);
    }
}

UTexture2D* UKOLoadSubsystem::ResolveFactoryIcon(FName FactoryId) const
{
    if (const TWeakObjectPtr<UTexture2D>* Cached = ResolvedFactoryIcons.Find(FactoryId))
    {
        if (Cached->IsValid())
        {
            return Cached->Get();
        }
    }

    const FKOFactoryRow* Row = FindFactoryRow(FactoryId);
    if (!Row)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveFactoryIcon: 알 수 없는 FactoryId '%s'."),
            *FactoryId.ToString());
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
            TEXT("UKOLoadSubsystem::ResolveFactoryIcon: '%s' 로드 실패 (FactoryId='%s')."),
            *Row->Icon.ToSoftObjectPath().ToString(), *FactoryId.ToString());
        return nullptr;
    }

    ResolvedFactoryIcons.Add(FactoryId, Texture);
    return Texture;
}

const FKOSkillRow* UKOLoadSubsystem::FindSkillRow(FName SkillId) const
{
    const FKOSkillRow* const* Found = SkillCache.Find(SkillId);
    return Found ? *Found : nullptr;
}

void UKOLoadSubsystem::GetAllSkillIds(TArray<FName>& Out) const
{
    SkillCache.GetKeys(Out);
}

UTexture2D* UKOLoadSubsystem::ResolveSkillIcon(FName SkillId) const
{
    if (const TWeakObjectPtr<UTexture2D>* Cached = ResolvedSkillIcons.Find(SkillId))
    {
        if (Cached->IsValid())
        {
            return Cached->Get();
        }
    }
    const FKOSkillRow* Row = FindSkillRow(SkillId);
    if (!Row)
    {
        UE_LOG(LogKOLoad, Warning,
            TEXT("UKOLoadSubsystem::ResolveSkillIcon: 알 수 없는 SkillId '%s'."),
            *SkillId.ToString());
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
            TEXT("UKOLoadSubsystem::ResolveSkillIcon: '%s' 로드 실패 (SkillId='%s')."),
            *Row->Icon.ToSoftObjectPath().ToString(), *SkillId.ToString());
        return nullptr;
    }

    ResolvedSkillIcons.Add(SkillId, Texture);
    return Texture;
}

const FKOSkillExecutionRow* UKOLoadSubsystem::FindSkillExecutionRow(FName SkillExId) const
{
    const FKOSkillExecutionRow* const* Found = SkillExecutionCache.Find(SkillExId);
    return Found? *Found: nullptr;
}

void UKOLoadSubsystem::GetAllSkillExecutionIds(TArray<FName>& Out) const
{
    SkillExecutionCache.GetKeys(Out);
}

