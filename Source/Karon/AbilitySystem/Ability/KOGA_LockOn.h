#pragma once

#include "CoreMinimal.h"
#include "KOGameplayAbilityBase.h"
#include "KOGA_LockOn.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKOGA_LockOn : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_LockOn();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
};
