#pragma once

#include "CoreMinimal.h"
#include "KOGA_AttackBase.h"
#include "Data/KOComboActionData.h"
#include "KOGA_Attack_Light.generated.h"


class UAbilityTask_PlayMontageAndWait;

UCLASS()
class KARON_API UKOGA_Attack_Light : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Attack_Light();
	
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
	
	virtual void InputPressed(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;
	
	virtual void OnGameplayEventReceived(FGameplayEventData Payload) override;
	
	// ─── Combo Data ───────────────────────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack|Combo")
	TObjectPtr<UDataTable> ComboDataTable;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Attack|Combo")
	FName InitialComboRowName;
    
	FName CurrentComboRowName;
    
	// ─── Combo Flow Control ───────────────────────────────────────────────────
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;
	
	void PlayComboMontage(FName RowName);
	
	UFUNCTION()
	void OnComboMontageCompleted();
    
	UFUNCTION()
	void OnComboMontageCancelled();
	
	UFUNCTION()
	void OnComboMontageBlendOut();
	
	bool bIsComboWindowOpen = false;
	
	EAttackInputType BufferedInput = EAttackInputType::None;

private:
	uint8 CurrentCombo = 1;
	uint8 MaxCombo = 3;
};
