// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Data/Type/KOEnemyType.h"
#include "Engine/DataAsset.h"
#include "KOEnemyDataAsset.generated.h"

class AKOBaseEnemy;
/**
 * 
 */
UCLASS()
class KARON_API UKOEnemyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(GetItemType(), GetFName());
	}
	
	virtual FPrimaryAssetType GetItemType() const
	{
		return FPrimaryAssetType("EnemyData");
	}
	
public:
	//Enemy Name
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	FGameplayTag EnemyName;
	
	//Enemy BP
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Data", meta=(AssetBundles="Enemy"))
	TSoftClassPtr<AKOBaseEnemy> EnemyBP;
	
	

	
};
