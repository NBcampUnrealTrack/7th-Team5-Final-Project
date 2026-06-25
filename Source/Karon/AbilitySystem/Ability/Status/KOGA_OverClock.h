#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_OverClock.generated.h"

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
		const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;
	
	UFUNCTION()
	void OnOverClockGaugeEmpty();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "OverClock")
	TSubclassOf<UGameplayEffect> OverClockBuffEffectClass;
	
	UPROPERTY(EditDefaultsOnly, Category = "OverClock")
	TSubclassOf<UGameplayEffect> OverClockDrainEffectClass;
	
private:
	FActiveGameplayEffectHandle BuffEffectHandle;
	FActiveGameplayEffectHandle DrainEffectHandle;
};
