#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/Attack/KOGA_AttackBase.h"
#include "KOGA_Attack_Normal.generated.h"

UCLASS()
class KARON_API UKOGA_Attack_Normal : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Attack_Normal();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
protected:
	UFUNCTION()
	void OnSocketSwap(FGameplayEventData EventData); 
	
	UFUNCTION()
	void OnTraceStart(FGameplayEventData EventData);
	
	UFUNCTION()
	void OnTraceEnd(FGameplayEventData EventData);
	
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();
};

