#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_Gurad.generated.h"


UCLASS()
class KARON_API UKOGA_Gurad : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
public:
	UKOGA_Gurad();
	
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
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnPerfectWindowStart(FGameplayEventData Data);
	
	UFUNCTION()
	void OnPerfectWindowEnd(FGameplayEventData Data);
	
	UFUNCTION() 
	void OnHitReceived(FGameplayEventData Data);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> GuardMontage;
	
private:
	bool bPerfectGuardWindowOpen = false; 
};
