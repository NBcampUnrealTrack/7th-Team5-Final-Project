#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/Attack/KOGA_AttackBase.h"
#include "KOGA_Normal_Skill.generated.h"

UCLASS()
class KARON_API UKOGA_Normal_Skill : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Normal_Skill();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo, 
		const FGameplayEventData* TriggerEventData
	) override;

protected:
	UFUNCTION()
	void OnTraceStart(FGameplayEventData Payload);
    
	UFUNCTION()
	void OnTraceEnd(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMontageCompleted();
	
	UFUNCTION()
	void OnMontageCancelled();
};
