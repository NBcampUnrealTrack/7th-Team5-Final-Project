// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KOEnemyGameplayAbility.h"
#include "AbilitySystem/Ability/Attack/KOGA_AttackBase.h"
#include "KOGA_EnemyCounterAttack.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKOGA_EnemyCounterAttack : public UKOEnemyGameplayAbility
{
	GENERATED_BODY()
	UKOGA_EnemyCounterAttack();
	
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
};
