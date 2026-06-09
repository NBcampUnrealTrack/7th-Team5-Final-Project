#pragma once

#include "CoreMinimal.h"
#include "KOGA_AttackBase.h"
#include "KOGA_Attack_Heavy.generated.h"


UCLASS()
class KARON_API UKOGA_Attack_Heavy : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Attack_Heavy();
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

private:
	UFUNCTION()
	void OnMontageEnded();
	
	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);
};
