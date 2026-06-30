// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_Parried.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKOGA_Parried : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Parried();
	
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
		bool bReplicateEndAbility, bool bWasCancelled
	) override;
	
private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();
		
	UFUNCTION()
	void OnRecoverTimeEnd();
	
public:
	FName ParriedSuccessSectionName = FName("ParriedSuccess");
	FName ParriedStandUpSectionName = FName("ParriedStandUp");
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> ParriedMontage;
	
private:
	FTimerHandle RecoverTimerHandle;
	
	float RecoverInterval = 5.f;

};
