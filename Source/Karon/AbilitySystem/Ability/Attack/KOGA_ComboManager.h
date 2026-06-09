#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "Data/KOComboActionData.h"
#include "KOGA_ComboManager.generated.h"

UCLASS()
class KARON_API UKOGA_ComboManager : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_ComboManager();
	
protected:
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
	
	virtual void InputPressed(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;
	
	UPROPERTY(EditDefaultsOnly, Category= "Combo|Data")
	TObjectPtr<UDataTable> ComboDataTable;
	
	UPROPERTY(EditDefaultsOnly, Category= "Combo|Data")
	FName InitialComboRowName;
	
private:
	FName CurrentComboRowName;
	bool bIsComboWindowOpen = false;
	EAttackInputType BufferedInput;
	
	UFUNCTION()
	void SendExecutionEvent(FName RowName);
	
	UFUNCTION()
	void OnComboWindowOpened(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnComboWindowClosed(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnLightInputReceived(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnHeavyInputReceived(FGameplayEventData Payload);
};
