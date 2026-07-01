#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_OverClock.generated.h"


class UAbilityTask_WaitGameplayEvent;
UCLASS()
class KARON_API UKOGA_OverClock : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_OverClock();
	
protected:
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
	
	UFUNCTION()
	void OnClockGain(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnOverClockStart(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnOverClockEnd(FGameplayEventData Payload);
	
protected:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GE_OverClockBuff;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GE_ClockDrain;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameplayEffect> GE_ClockGain;
	
	UPROPERTY(EditDefaultsOnly, Category = "OverClock|Effect")
	FGameplayTag ActivationCueTag;
	
private:
	UPROPERTY() 
	TObjectPtr<UAbilityTask_WaitGameplayEvent> ClockGainTask;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> OverClockStartTask;
	
	UPROPERTY() 
	TObjectPtr<UAbilityTask_WaitGameplayEvent> OverClockEndTask;
	
	FActiveGameplayEffectHandle BuffEffectHandle;
	FActiveGameplayEffectHandle DrainEffectHandle;
	
};
