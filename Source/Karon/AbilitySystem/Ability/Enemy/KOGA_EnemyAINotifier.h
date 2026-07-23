#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_EnemyAINotifier.generated.h"


UCLASS()
class KARON_API UKOGA_EnemyAINotifier : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_EnemyAINotifier();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
protected:
	UFUNCTION()
	void OnDamageEventReceived(FGameplayEventData Payload);
};
