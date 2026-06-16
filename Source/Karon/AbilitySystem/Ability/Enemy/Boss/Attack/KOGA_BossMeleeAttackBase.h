#pragma once

#include "CoreMinimal.h"
#include "KOGA_BossAttackBase.h"

#include "KOGA_BossMeleeAttackBase.generated.h"

class UAbilityTask_WaitGameplayEvent;

UCLASS()
class KARON_API UKOGA_BossMeleeAttackBase : public UKOGA_BossAttackBase
{
	GENERATED_BODY()

public:
	UKOGA_BossMeleeAttackBase();
	
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
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitHitEventTask;
 
	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData EventData);
};
