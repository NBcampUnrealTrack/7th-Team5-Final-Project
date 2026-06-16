#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_Utility_DrawWeapon.generated.h"

//	Draw  - 칼집 -> 손
// Sheathe - 손 -> 칼집 
// Pick up - 월드 -> 인벤토리  
// Drop - 인벤토리 -> 월드 
class UKOWeaponDefinition;

UCLASS()
class KARON_API UKOGA_Utility_DrawWeapon : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
public:
	UKOGA_Utility_DrawWeapon();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
private:
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnDrawAttachEvent(FGameplayEventData Payload);
};
