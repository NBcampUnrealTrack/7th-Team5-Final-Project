#pragma once

#include "CoreMinimal.h"
#include "KOGA_AttackBase.h"
#include "KOGA_Attack_Light.generated.h"


UCLASS()
class KARON_API UKOGA_Attack_Light : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Attack_Light();
	
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
