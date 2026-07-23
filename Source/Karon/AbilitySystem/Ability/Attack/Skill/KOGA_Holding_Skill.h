#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/Attack/KOGA_AttackBase.h"
#include "KOGA_Holding_Skill.generated.h"

class UAbilityTask_PlayMontageAndWait;

UCLASS()
class KARON_API UKOGA_Holding_Skill : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Holding_Skill();
	
protected:
	virtual void ActivateAbility(
		 FGameplayAbilitySpecHandle Handle,
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
	
	virtual void InputReleased(
	   const FGameplayAbilitySpecHandle Handle,
	   const FGameplayAbilityActorInfo* ActorInfo,
	   const FGameplayAbilityActivationInfo ActivationInfo
	) override;
	
	virtual void ExecuteFinisher();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Holding|Animation")
	FKOAttackMontageData HoldMontageData;
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Holding|Animation")
	FKOAttackMontageData FinisherMontageData; 
    
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Holding|Time")
	float MaxHoldTime;
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Holding|Damage")
	TArray<FKODamageEffectData> HoldDamageEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Holding|Damage")
	TArray<FKOEffectData> AdditionalHoldDamageEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Holding|Damage")
	TArray<FKODamageEffectData> FinishDamageEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Holding|Damage")
	TArray<FKOEffectData> AdditionalFinishDamageEffect;

private:
	FTimerHandle MaxHoldTimerHandle;
	bool bIsHolding;
	bool bIsFinisherActive;
    
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> HoldMontageTask;
    
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> FinisherMontageTask; 
    
	UFUNCTION()
	void OnHoldMontageInterrupted();
    
	UFUNCTION()
	void OnFinisherMontageCompleted();
    
	UFUNCTION()
	void OnFinisherMontageInterrupted();
 
	UFUNCTION()
	void OnTraceStart(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnTraceEnd(FGameplayEventData Payload);
};
