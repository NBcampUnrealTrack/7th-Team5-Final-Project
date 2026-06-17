#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_Charge_Attack_Base.generated.h"


UCLASS()
class KARON_API UKOGA_Charge_Attack_Base : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Charge_Attack_Base();
	
protected:
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
		bool bReplicateEndAbility, bool bWasCancelled
	) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charge|Animation")
	UAnimMontage* ChargeMontage;
	
	UPROPERTY()
	float MaxChargeTime;
	
	UPROPERTY()
	float ChargeTickRate;
	
	virtual void ExecuteAttack(float ChargePercentage);
	
private:
		
	FTimerHandle ChargeTimerHandle;
	float CurrentChargeTime;
	bool bIsCharging;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> ChargeMontageTask;
	
	UFUNCTION()
	void OnInputReleased(float TimeHeld);
	
	UFUNCTION()
	void OnChargeMontageInterrupted();
	
	UFUNCTION()
	void OnChargeTick();
	
	void StartCharging();
	void StopCharging();

};
