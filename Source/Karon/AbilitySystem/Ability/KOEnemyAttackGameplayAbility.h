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
	
protected:
	UPROPERTY()
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
