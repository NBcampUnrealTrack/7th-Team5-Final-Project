// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/KODataTableTypes.h"
#include "KOLoadSubsystem.generated.h"

class UTexture2D;
class UStaticMesh;
class AKOBaseBuilding;
class UKOWeaponDefinition;

/**
 * UKOLoadSubsystem
 * 동작 : [GameInstance] Initialize() → LoadAll()
 *
 * [API]
 * FindItemRow / FindFactoryRow / FindRecipeRow — 모두 RowName(FName) 키 기반.
 * ResolveItemIcon / ResolveBuildingClass — 소프트 레퍼런스 지연 로드.
 */
UCLASS()
class KARON_API UKOLoadSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    static UKOLoadSubsystem* Get(const UObject* WorldContext);

    const FKOItemRow*    FindItemRow(FName ItemId)         const;
    const FKOFactoryRow* FindFactoryRow(FName FactoryId)   const;
    const FKORecipeRow*  FindRecipeRow(FName RecipeId)     const;
    const FKOEquipmentRow* FindEquipmentRow(FName EquipmentId) const;
    const FKOEquipmentRow* FindEquipmentRowByItemTag(FGameplayTag ItemTag) const;
    const FKOSkillRow* FindSkillRow(FName SkillId) const;
    const FKOSkillExecutionRow* FindSkillExecutionRow(FName SkillExId) const;

    /** ItemTag로 ItemId(RowName) 역조회. 매핑이 없으면 NAME_None. */
    FName FindItemIdByTag(FGameplayTag ItemTag) const;
    
    /** Equipment DT에 등록된 ItemTag인지 확인 */
    bool IsEquipmentItem(FGameplayTag ItemTag) const;

    void GetAllItemIds(TArray<FName>& Out)     const;
    void GetAllFactoryIds(TArray<FName>& Out)  const;
    void GetAllRecipeIds(TArray<FName>& Out)   const;
    void GetAllEquipmentIds(TArray<FName>& Out) const;
    void GetAllSkillIds(TArray<FName>& Out) const;
    void GetAllSkillExecutionIds(TArray<FName>& Out) const;
    /** 제작 가능한 EquipmentId 목록 반환. */
    void GetCraftableEquipmentIds(TArray<FName>& Out) const;

    /**
     * 빌드 메뉴에 노출할 FactoryId 목록을 SortOrder 오름차순으로 반환.
     * Query.Category가 None이면 카테고리 제한 없음.
     * RequiredUnlockTags가 비어있지 않은 Row는 Query.OwnedUnlocks가 모두 포함해야 통과.
     */
    void GetBuildableFactoryIds(const FKOBuildMenuQuery& Query, TArray<FName>& Out) const;

    /** Factory Row의 Icon 소프트 레퍼런스를 동기 로드 (캐싱). */
    UTexture2D* ResolveFactoryIcon(FName FactoryId) const;
    UTexture2D* ResolveItemIcon(FName ItemId) const;
    UTexture2D* ResolveSkillIcon(FName SkillId) const;
    
    /** Item Row의 WorldMesh 소프트 레퍼런스를 동기 로드 (캐싱). 미지정/실패 시 nullptr. */
    UStaticMesh*  ResolveItemMesh(FName ItemId) const;
    
    /** Factory Row의 BuildingClass 소프트 레퍼런스를 동기 로드 */
    UClass* ResolveBuildingClass(FName FactoryId) const;
    
    /** WeaponDefinition Row의 ItemId 소프트 레퍼런스를 동기 로드 */
    UKOWeaponDefinition* ResolveWeaponDefinitionByItemId(FName ItemId) const;
    
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

    TMap<FName, const FKOItemRow*>           ItemCache;
    TMap<FName, const FKOFactoryRow*>        FactoryCache;
    TMap<FName, const FKORecipeRow*>         RecipeCache;
    TMap<FName, const FKOEquipmentRow*>      EquipmentCache;
    TMap<FName, const FKOSkillRow*>          SkillCache;
    TMap<FName, const FKOSkillExecutionRow*> SkillExecutionCache;

    /** ItemTag → ItemId 역인덱스. LoadAll에서 ItemCache 채운 직후 빌드. */
    TMap<FGameplayTag, FName>         ItemTagToId;
    
    /** Equipment ItemTag → EquipmentId(RowName) 역인덱스 */
    TMap<FGameplayTag, FName>         EquipmentTagToId;
    
    // 약한 참조 관련 Mutable 처리
    mutable TMap<FName, TWeakObjectPtr<UTexture2D>>  ResolvedIcons;
    mutable TMap<FName, TWeakObjectPtr<UStaticMesh>>  ResolvedItemMeshes;
    mutable TMap<FName, TWeakObjectPtr<UClass>>      ResolvedBuildingClasses;
    mutable TMap<FName, TWeakObjectPtr<UTexture2D>>  ResolvedFactoryIcons;
    mutable TMap<FName, TWeakObjectPtr<UTexture2D>>  ResolvedSkillIcons;
};
