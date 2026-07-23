// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h" 
#include "Data/Type/KOEnemyType.h"
#include "KOEnemyDataSubsystem.generated.h"

/** 
 * 
 */
UCLASS()
class KARON_API UKOEnemyDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	static UKOEnemyDataSubsystem* Get(UObject* WorldContext);
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	// 특정 에너미,스킬 태그로 데이터를 빠르게 검색
	float GetSkillData(FEnemySkillInfo EnemySkillInfo);
	
	// 특정 에너미 태그, 레벨로 데이터를 빠르게 검색
	FEnemyInfo* GetEnemyData(FEnemyNameLevelInfo EnemyNameLevelInfo);
	
	// 특정 에너미 태그, 레벨로 드랍아이템을 빠르게 검색
	TArray<FEnemyDropItemInfo>* GetEnemyDropItemArray(FEnemyNameLevelInfo EnemyNameLevelInfo);

private:
	void InitEnemySkillSettings();
	void InitEnemyDataSettings();
	void InitEnemyDropItemSettings();
	
private:
	UPROPERTY()
	TMap<FEnemySkillInfo,float> SkillDataMap;
	
	UPROPERTY()
	TMap<FEnemyNameLevelInfo,FEnemyInfo> EnemyDataMap;
	
	UPROPERTY()
	TMap<FEnemyNameLevelInfo,FEnemyDropItemArrayWrapper> EnemyDropItemMap;
};
