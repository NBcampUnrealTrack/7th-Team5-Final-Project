#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_AttackBase.generated.h"


UCLASS()
class KARON_API UKOGA_AttackBase : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_AttackBase();
	
public:	
	// ─── Ability Life Cycle ───────────────────────────────────────────────────
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	)	override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle, 
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, 
		bool bWasCancelled
	) override;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Event")
	FGameplayTag AttackEventTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Debug")
	bool bShowDebug = true; 
};
