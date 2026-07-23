// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "KODataRegistrySettings.generated.h"

class UDataTable;

/**
 * UKODataRegistrySettings
 * 게임 시작 시 로드할 DataTable 목록을 보관하는 프로젝트 전역 설정.
 * Project Settings → Karon → KO Data Registry 에서 편집
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "KO Data Registry"))
class KARON_API UKODataRegistrySettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    virtual FName GetCategoryName() const override { return TEXT("Karon"); }
    
    UPROPERTY(Config, EditAnywhere, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> ItemTables;
    
    UPROPERTY(Config, EditAnywhere, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> FactoryTables;
    
    UPROPERTY(Config, EditAnywhere, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> RecipeTables;
    
    UPROPERTY(EditDefaultsOnly, Config, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> EquipmentTables;
    
    UPROPERTY(Config, EditAnywhere, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> SkillTables;

    UPROPERTY(Config, EditAnywhere, Category = "KO|Data Registry")
    TArray<TSoftObjectPtr<UDataTable>> SkillExecutionTables;

};
