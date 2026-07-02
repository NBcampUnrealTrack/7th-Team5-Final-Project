#pragma once

#include "CoreMinimal.h"
#include "KOGA_AttackBase.h"
#include "KOGA_Attack_Combo.generated.h"

class UGameplayEffect;
class UAbilityTask_PlayMontageAndWait;

UCLASS()
class KARON_API UKOGA_Attack_Combo : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Attack_Combo();
	
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
	)override;
	
	virtual void InputPressed(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;

protected:	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "OverClock")
	TSubclassOf<UGameplayEffect> OverClockGainEffectClass;
	
	void PlayComboMontage();
	
	UFUNCTION()
	void OnMontageCompleted();
	
	UFUNCTION()
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnTraceStart(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnTraceEnd(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnComboWindowOpened(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnComboWindowClosed(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void OnReceiveTransition(FGameplayEventData Payload);

private:
	int32 ComboIndex = 0;
	bool bNextComboRequested = false;
	bool bComboWindowOpen = false;
	bool bIsTransitioning = false; 
	float GroggyRadius=300.f;
	FName GroggyLoopName=FName("ParriedLoop");
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;
	
};
