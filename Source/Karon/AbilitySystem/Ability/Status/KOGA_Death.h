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
	
private:
	UFUNCTION()
	void OnReceiveEvent(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMontageCompleted();
	
	UFUNCTION()
	void OnMontageInterrupted();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	UAnimMontage* DeathMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> GE_Death;
	
private:
	TWeakObjectPtr<AActor> CachedInstigator;

};

