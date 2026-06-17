// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h" 
#include "Data/Type/KOEnemyType.h"
#include "KOEnemySkillSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKOEnemySkillSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	static UKOEnemySkillSubsystem* Get(UObject* WorldContext);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	// 특정 에너미,스킬 태그로 데이터를 빠르게 검색
	float GetSkillData(FEnemySkillInfoTag EnemyTag);

private:
	UPROPERTY()
	TMap<FEnemySkillInfoTag,float> SkillDataMap;
};
