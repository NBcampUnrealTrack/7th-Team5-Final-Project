#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_StaminaExhausted.generated.h"


UCLASS()
class KARON_API UKOGA_StaminaExhausted : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_StaminaExhausted();
	
	virtual void ActivateAbility(
	  const FGameplayAbilitySpecHandle Handle,
	  const FGameplayAbilityActorInfo* ActorInfo,
	  const FGameplayAbilityActivationInfo ActivationInfo,
	  const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	UFUNCTION()
	void OnStaminaFull(FGameplayEventData Data);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Exhausted")
	TSubclassOf<UGameplayEffect> ExhaustedEffect;
	
	UPROPERTY(EditDefaultsOnly, Category = "Exhausted")
	float ExhaustedSpeed = 100.f; 
	

	FActiveGameplayEffectHandle ExhaustedEffectHandle;
};


