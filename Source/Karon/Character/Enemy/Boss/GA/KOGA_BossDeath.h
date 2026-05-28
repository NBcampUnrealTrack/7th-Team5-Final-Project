#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "KOGA_BossDeath.generated.h"

UCLASS()
class KARON_API UKOGA_BossDeath : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UKOGA_BossDeath();
 
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
};
