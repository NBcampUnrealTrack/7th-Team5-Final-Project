// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Data/Type/KOEnemyType.h"
#include "Engine/DataAsset.h"
#include "KOEnemyDataAsset.generated.h"

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
	//ID
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	FString EnemyID;
	//적 스탯 구조체
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy")
	FEnemyStat EnemyStat;
	
	
	
	//스켈레탈 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Data", meta=(AssetBundles="Enemy"))
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
	//애님몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Data", meta=(AssetBundles="Enemy"))
	TMap<FName,TSoftObjectPtr<UAnimMontage>> AnimMontages;
	//Walk/Run 블렌드 스페이스
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Data", meta=(AssetBundles="Enemy"))
	TSoftObjectPtr<UBlendSpace> EnemyBlendSpace;
	//적의 ABP
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Data", meta=(AssetBundles="Enemy"))
	TSoftClassPtr<UAnimInstance> EnemyAnimationBlueprint;
	//발사체의 스켈레탈 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Projectile", meta=(AssetBundles="Enemy"))
	TSoftObjectPtr<UStaticMesh> ProjectileStaticMesh;

	//히트 파티클
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Data", meta=(AssetBundles="Enemy"))
	TSoftObjectPtr<UParticleSystem> HitParticle;
	//BehaviorTree
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Data", meta=(AssetBundles="Enemy"))
	TSoftObjectPtr<UBehaviorTree> EnemyBT;
	//오버레이머티리얼
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy|Data", meta=(AssetBundles="Enemy"))
	TSoftObjectPtr<UMaterial> EnemyMaterial;
	
	//AIControllerClass
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI", meta = (AssetBundles = "Enemy"))
	TSoftClassPtr<AAIController> AIControllerClass;
	

	
};
