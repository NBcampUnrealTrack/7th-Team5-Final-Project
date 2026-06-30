#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_Guard.generated.h"


UCLASS()
class KARON_API UKOGA_Guard : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
public:
	UKOGA_Guard();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled
	) override;
	
protected:
	UFUNCTION()
	void OnMontageCompleted();
	
	UFUNCTION()
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnGuardFailed(FGameplayEventData Data);
	
	UFUNCTION()
	void OnGuardStart(FGameplayEventData Data);
	
	UFUNCTION()
	void OnGuardEnd(FGameplayEventData Data);
	
	UFUNCTION()
	void OnPerfectWindowStart(FGameplayEventData Data);
	
	UFUNCTION()
	void OnPerfectWindowEnd(FGameplayEventData Data);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> GuardMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GE_Guard_Init; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GE_Guard_Reset; 
	
	FActiveGameplayEffectHandle GuardEffectHandle; 
};
