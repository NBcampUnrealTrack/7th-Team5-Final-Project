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
 * DataTable 기반 런타임 데이터(아이템, 공장, 레시피)를 로드·캐싱하는
 * GameInstance 수명 서브시스템.
 *
 * 라이프사이클
 * -----------
 * Initialize() → LoadAll()
 *   1. UKODataRegistrySettings(DeveloperSettings)에서 테이블 목록을 조회한다.
 *   2. 설정에 나열된 UDataTable을 각각 동기 로드한다.
 *   3. 행을 순회하여 세 개의 런타임 TMap 캐시에 색인한다.
 *
 * 모든 로딩은 동기(LoadSynchronous)로 이루어지며, World 생성 이전인
 * GameInstance 초기화 단계에서 완료되므로 인게임 히치가 없다.
 *
 * 키 정책
 * -------
 * - 아이템은 RowName(FName)을 단일 식별자로 사용한다.
 * - 공장/레시피는 행 내부의 FGameplayTag 필드를 식별자로 사용한다.
 *
 * 조회 API
 * --------
 * FindItemRow(FName) / FindFactoryRow / FindRecipeRow — 키 기반 O(1) 캐시 조회.
 * ResolveItemIcon / ResolveItemMesh — 소프트 에셋 포인터 지연 동기 로드.
 *   결과는 TWeakObjectPtr 맵에 캐싱되어 중복 로드를 방지하면서도 GC 수거를 허용한다.
 */
UCLASS()
class KARON_API UKOLoadSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

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

    /** 아이템 테이블 전용 인덱싱 (RowName을 키로 사용) */
    void IndexItemTables(const TArray<TSoftObjectPtr<UDataTable>>& SoftTables);

    /** 태그 필드를 키로 갖는 테이블용 범용 인덱서 */
    template<typename TRow>
    void IndexTableRowsByTag(
        const TArray<TSoftObjectPtr<UDataTable>>& SoftTables,
        TMap<FGameplayTag, const TRow*>&          OutCache,
        TFunctionRef<FGameplayTag(const TRow&)>   GetTag,
        const TCHAR*                              TableKind);

    // DataTable 강한 참조로 GC 수거 방지
    UPROPERTY()
    TArray<TObjectPtr<UDataTable>> LoadedTables;

    TMap<FName,         const FKOItemRow*>    ItemCache;
    TMap<FGameplayTag,  const FKOFactoryRow*> FactoryCache;
    TMap<FGameplayTag,  const FKORecipeRow*>  RecipeCache;

    // 약한 참조 관련 Mutable 처리
    mutable TMap<FName, TWeakObjectPtr<UTexture2D>>  ResolvedIcons;
    mutable TMap<FName, TWeakObjectPtr<UStaticMesh>> ResolvedMeshes;
};
