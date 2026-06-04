// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KOGameplayAbilityBase.h"
#include "KOEnemyAttackGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKOEnemyAttackGameplayAbility : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOEnemyAttackGameplayAbility();
	
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayTagContainer* SourceTags = nullptr, 
		const FGameplayTagContainer* TargetTags = nullptr, 
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr
	) const override;
	
	virtual void ActivateAbility(
	   const FGameplayAbilitySpecHandle Handle,
	   const FGameplayAbilityActorInfo* ActorInfo,
	   const FGameplayAbilityActivationInfo ActivationInfo,
	   const FGameplayEventData* TriggerEventData
   ) override;
	
	virtual void EndAbility(
	   const FGameplayAbilitySpecHandle Handle,
	   const FGameplayAbilityActorInfo* ActorInfo,
	   const FGameplayAbilityActivationInfo ActivationInfo,
	   bool bReplicateEndAbility,
	   bool bWasCancelled
   ) override;

private:
	UFUNCTION()
	void OnMontageCompleted();
	
	UFUNCTION()
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnNotifyHitEvent(FGameplayEventData HitGameplayEventData);

	//몽타주의 AnimNotify에서 변경되는 변수
public:
	//이전 Notify틱의 소켓의 위치
	UPROPERTY()
	FVector PresentAttackSocketLocation=FVector::ZeroVector;
	//현재 Notify틱의 소켓의 위치
	UPROPERTY()
	FVector CurrentAttackSocketLocation=FVector::ZeroVector;
	//현재 Notify에서 플레이어를 타격하였는지 여부(중복 타격 방지, 싱글플레이)
	UPROPERTY()
	bool bIsAttacked=false;

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> AnimMontage=nullptr;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TMap<TSubclassOf<UGameplayEffect>,float> DebuffEffectClassMap;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	TSubclassOf<UGameplayEffect> CoolTimeEffectClass;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	float CoolTime=0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	float SKillAttackSpeed=1.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skill")
	FGameplayTag CoolTimeTag=FGameplayTag::EmptyTag;
	
};
