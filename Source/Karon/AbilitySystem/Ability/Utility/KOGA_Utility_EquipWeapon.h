#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_Utility_EquipWeapon.generated.h"

UCLASS()
class KARON_API UKOGA_Utility_EquipWeapon : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Utility_EquipWeapon();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
};
