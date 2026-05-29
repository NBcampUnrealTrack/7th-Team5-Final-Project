#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_Death.generated.h"

UCLASS()
class KARON_API UKOGA_Death : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Death();
	
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
	
private:
	UFUNCTION()
	void OnMontageCompleted();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	UAnimMontage* Montage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> GE_Death;
};

