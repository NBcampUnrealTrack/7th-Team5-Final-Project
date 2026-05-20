// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "KODataRegistrySettings.generated.h"

class UDataTable;

/**
 * UKODataRegistrySettings
 *
 * 게임 시작 시 로드할 DataTable 목록을 보관하는 프로젝트 전역 설정.
 * Project Settings → Karon → KO Data Registry 에서 편집하며, 값은
 * DefaultGame.ini의 [/Script/Karon.KODataRegistrySettings] 섹션에 저장된다.
 *
 * UKOLoadSubsystem::LoadAll()이 GetDefault<UKODataRegistrySettings>()를 통해
 * 본 설정을 조회하고 각 테이블을 동기 로드하여 런타임 캐시에 색인한다.
 *
 * 테이블은 소프트 포인터로 보관하여 설정 자체를 경량으로 유지한다.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "KO Data Registry"))
class KARON_API UKODataRegistrySettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    virtual FName GetCategoryName() const override { return TEXT("Karon"); }

    /** FKOItemRow 행을 가진 DataTable 목록. 여러 테이블 추가 가능하며 전부 하나의 캐시로 병합된다. */
    UPROPERTY(Config, EditAnywhere, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> ItemTables;

    /** FKOFactoryRow 행을 가진 DataTable 목록 */
    UPROPERTY(Config, EditAnywhere, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> FactoryTables;

    /** FKORecipeRow 행을 가진 DataTable 목록 */
    UPROPERTY(Config, EditAnywhere, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> RecipeTables;
};
