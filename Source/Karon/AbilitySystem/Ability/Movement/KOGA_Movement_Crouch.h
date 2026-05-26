#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_Movement_Crouch.generated.h"

UCLASS()
class KARON_API UKOGA_Movement_Crouch : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Movement_Crouch();
	
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, 
		const FGameplayTagContainer* TargetTags = nullptr, 
		FGameplayTagContainer* OptionalRelevantTags = nullptr
	) const override;
	
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
	
	virtual void InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;
	
protected:
	UFUNCTION()
	void OnMontageCompleted();
	
	UFUNCTION()
	void OnMontageCancelled();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Effect")
	TSubclassOf<UGameplayEffect> SpeedDebuff; 
	
	
};
