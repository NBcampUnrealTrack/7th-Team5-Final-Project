#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_Tick.h"
#include "KOGA_Guard.generated.h"


UCLASS()
class KARON_API UKOGA_Guard : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
public:
	UKOGA_Guard();
	
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
	void OnMontageCompleted();
	
	UFUNCTION()
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnStaminaChanged();
	
	UFUNCTION()
	void OnGuardSuccess(FGameplayEventData Data);
	
	UFUNCTION()
	void OnGuardFailed(FGameplayEventData Data);
	
	UFUNCTION()
	void OnGuardStart(FGameplayEventData Data);
	
	UFUNCTION()
	void OnGuardEnd(FGameplayEventData Data);
	
	UFUNCTION()
	void OnPerfectWindowStart(FGameplayEventData Data);
	
	UFUNCTION()
	void OnPerfectWindowEnd(FGameplayEventData Data);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> GuardMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GE_Guard_Init; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> GE_Guard_Reset; 
	
	FActiveGameplayEffectHandle GuardEffectHandle; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float LaunchAmount = 500.f;

private:
	UFUNCTION()
	void CheckGuardLifetime(float DeltaTime);

	TObjectPtr<UAbilityTask_Tick> WatchdogTask;
	float ActivationTime = 0.f;
	bool bWatchdogLogged = false;
	static constexpr float MaxExpectedDuration = 10.f;
};
