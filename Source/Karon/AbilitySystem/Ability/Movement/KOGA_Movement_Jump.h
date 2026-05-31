#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "UObject/Object.h"
#include "KOGA_Movement_Jump.generated.h"

UCLASS()
class KARON_API UKOGA_Movement_Jump : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Movement_Jump();
	
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
	
	UFUNCTION()
	void OnCharacterLanded(EMovementMode NewMovementMode);
};
