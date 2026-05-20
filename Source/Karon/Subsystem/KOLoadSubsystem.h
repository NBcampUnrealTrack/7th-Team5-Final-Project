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
 * 조회 API
 * --------
 * FindItemRow / FindFactoryRow / FindRecipeRow — 태그 기반 O(1) 캐시 조회.
 * ResolveItemIcon / ResolveItemMesh — 소프트 에셋 포인터 지연 동기 로드.
 *   결과는 TWeakObjectPtr 맵에 캐싱되어 중복 로드를 방지하면서도 GC 수거를 허용한다.
 */
UCLASS()
class KARON_API UKOLoadSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // ─── USubsystem ──────────────────────────────────────────────────────────
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ─── Row Lookup API ───────────────────────────────────────────────────────

    /** 태그에 해당하는 FKOItemRow 캐시 포인터를 반환한다. 등록되지 않은 태그면 nullptr. */
    const FKOItemRow*    FindItemRow(FGameplayTag ItemTag)    const;

    /** 태그에 해당하는 FKOFactoryRow 캐시 포인터를 반환한다. 등록되지 않은 태그면 nullptr. */
    const FKOFactoryRow* FindFactoryRow(FGameplayTag FactoryTag) const;

    /** 태그에 해당하는 FKORecipeRow 캐시 포인터를 반환한다. 등록되지 않은 태그면 nullptr. */
    const FKORecipeRow*  FindRecipeRow(FGameplayTag RecipeTag)  const;

    // ─── Asset Resolve API ────────────────────────────────────────────────────

    /**
     * 아이템 태그의 UTexture2D 아이콘을 반환한다.
     * 최초 호출 시 소프트 포인터를 동기 로드하며, 이후 호출은 TWeakObjectPtr 캐시에서 반환한다.
     * 알 수 없는 태그이거나 소프트 경로가 null이면 nullptr을 반환한다.
     */
    UTexture2D*   ResolveItemIcon(FGameplayTag ItemTag) const;

    /**
     * 아이템 태그의 UStaticMesh를 반환한다.
     * ResolveItemIcon과 동일한 지연 로드 + 약한 참조 캐시 패턴을 사용한다.
     * 알 수 없는 태그이거나 소프트 경로가 null이면 nullptr을 반환한다.
     */
    UStaticMesh*  ResolveItemMesh(FGameplayTag ItemTag)  const;

    // ─── Enumeration API ──────────────────────────────────────────────────────

    /** 등록된 모든 아이템 태그를 Out에 채운다. */
    void GetAllItemTags(TArray<FGameplayTag>& Out)    const;

    /** 등록된 모든 공장 태그를 Out에 채운다. */
    void GetAllFactoryTags(TArray<FGameplayTag>& Out) const;

    /** 등록된 모든 레시피 태그를 Out에 채운다. */
    void GetAllRecipeTags(TArray<FGameplayTag>& Out)  const;

private:
    // ─── Internal Loading ─────────────────────────────────────────────────────

    /** 전체 동기 로드 파이프라인을 실행한다. Initialize()에서 호출된다. */
    void LoadAll();

    /**
     * SoftTables의 각 DataTable을 동기 로드하고, TRow 타입 행을 순회하여
     * GetTag가 반환하는 태그 키로 OutCache에 삽입한다.
     * HardRefs에는 로드된 UDataTable* 강한 참조를 쌓아 호출 스코프 내 GC 수거를 방지한다.
     *
     * @param SoftTables  로드할 DataTable 소프트 포인터 배열
     * @param OutCache    채울 대상 맵
     * @param GetTag      행에서 FGameplayTag 키를 추출하는 펑터
     * @param TableKind   로그 메시지용 종류 문자열 (예: TEXT("Item"))
     * @param HardRefs    로드 중 GC를 방지하기 위해 호출자가 소유하는 배열
     */
    template<typename TRow>
    void IndexTableRows(
        const TArray<TSoftObjectPtr<UDataTable>>& SoftTables,
        TMap<FGameplayTag, TRow>&                 OutCache,
        TFunctionRef<FGameplayTag(const TRow&)>   GetTag,
        const TCHAR*                              TableKind,
        TArray<UObject*>&                         HardRefs);

    // ─── 런타임 캐시 ─────────────────────────────────────────────────────────
    // UPROPERTY가 아닌 일반 TMap인 이유:
    //   a) FKOItemRow 등은 FTableRowBase 구조체로 UObject가 아니다.
    //   b) UObject* 캐시는 GC를 허용하기 위해 TWeakObjectPtr을 사용한다.

    TMap<FGameplayTag, FKOItemRow>    ItemCache;
    TMap<FGameplayTag, FKOFactoryRow> FactoryCache;
    TMap<FGameplayTag, FKORecipeRow>  RecipeCache;

    // 해석된 에셋의 약한 참조 캐시 — const Resolve 함수에서 수정 가능하도록 mutable 선언
    mutable TMap<FGameplayTag, TWeakObjectPtr<UTexture2D>>  ResolvedIcons;
    mutable TMap<FGameplayTag, TWeakObjectPtr<UStaticMesh>> ResolvedMeshes;
};
