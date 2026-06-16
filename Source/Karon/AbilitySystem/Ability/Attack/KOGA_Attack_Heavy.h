#pragma once

#include "CoreMinimal.h"
#include "KOGA_AttackBase.h"
#include "KOGA_Attack_Heavy.generated.h"

class UGameplayEffect;
class UAbilityTask_PlayMontageAndWait;

UCLASS()
class KARON_API UKOGA_Attack_Heavy : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Attack_Heavy();
	
protected:
	virtual bool CanActivateAbility(
	   const FGameplayAbilitySpecHandle Handle,
	   const FGameplayAbilityActorInfo* ActorInfo, 
	   const FGameplayTagContainer* SourceTags,
	   const FGameplayTagContainer* TargetTags, 
	   FGameplayTagContainer* OptionalRelevantTags
	) const override;
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;

	virtual void InputPressed(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo
	) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Data")
	TObjectPtr<UDataTable> ComboDataTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack|Data")
	FName WeaponRowName = FName("DefaultWeapon");
	
private:
	
	int32 CurrentComboIndex;
	int32 MaxComboCount;
	bool bIsComboQueued;
	bool bIsInputBufferOpen;
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> ComboMontage;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;
	
	UFUNCTION()
	void PlayNextComboSection();
	
	UFUNCTION()
	void OnComboWindowReceived(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMontageEnded();
	
	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnInputBufferOpened(FGameplayEventData Payload);
};
