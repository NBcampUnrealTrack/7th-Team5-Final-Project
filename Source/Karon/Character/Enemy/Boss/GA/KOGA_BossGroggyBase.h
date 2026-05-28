// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "KOGA_BossGroggyBase.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKOGA_BossGroggyBase : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UKOGA_BossGroggyBase();
 
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
 
protected:
	// 그로기 유지 시간 (BP에서 설정)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Groggy")
	float GroggyDuration = 5.f;
 
private:
	FTimerHandle GroggyTimerHandle;
 
	void OnGroggyTimerEnd();
};
