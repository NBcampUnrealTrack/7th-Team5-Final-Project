// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "KOEnemyDeveloperSettings.generated.h"

/**
 * 프로젝트 세팅에서 노출되는 Enemy Setting 입니다.
 */
UCLASS(Config=Game, DefaultConfig, meta = (DisplayName = "Enemy Settings"))
class KARON_API UKOEnemyDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> SkillMulDataTable;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> EnemyDataTable;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Data")
	TSoftObjectPtr<UDataTable> EnemyDropItemDataTable;
};
