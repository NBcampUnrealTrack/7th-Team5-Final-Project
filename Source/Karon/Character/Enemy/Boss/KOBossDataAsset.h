#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KOBossDataAsset.generated.h"
 
class UGameplayAbility;
class UGameplayEffect;

UCLASS()
class KARON_API UKOBossDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
 
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Info")
	FName BossName;
 
	// 무거운 에셋 (비동기 로드)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftClassPtr<UAnimInstance> AnimInstance;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset")
	TSoftObjectPtr<USoundBase> BossBGM;
 
	// 스탯 (동기 로드)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MaxHealth = 5000.f;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float MoveSpeed = 300.f;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float AttackPower = 120.f;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float Defense = 30.f;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats")
	float AttackSpeed = 1.f;
 
	// GA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TArray<TSubclassOf<UGameplayAbility>> BossAbilities;
 
	// 초기화 GE
	// 스탯 초기값을 GE로 주입할 경우 사용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> InitStatEffect;
};
