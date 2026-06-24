#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_Movement_Sprint.generated.h"


class AKOHeroCharacter;
class UCharacterMovementComponent;

UCLASS()
class KARON_API UKOGA_Movement_Sprint : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Movement_Sprint();
	
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo, 
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr, 
		OUT FGameplayTagContainer* OptionalRelevantTags = nullptr
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

public:
	UFUNCTION()
	void OnStaminaChanged();
	
	void TryStartGraceTimer();
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> SprintEffect;
	
	FActiveGameplayEffectHandle SprintEffectHandle; 

protected:
	
	FTimerHandle GraceTimer; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grace")
	float GraceTime = 1.f; 
	
protected:
	UPROPERTY()
	TObjectPtr<AKOHeroCharacter> CachedCharacter;
	
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CachedMovement;
};
