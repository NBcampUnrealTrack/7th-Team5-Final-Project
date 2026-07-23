#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/Attack/KOGA_AttackBase.h"
#include "KOGA_Charge_Skill.generated.h"

class UAbilityTask_PlayMontageAndWait;

UCLASS()
class KARON_API UKOGA_Charge_Skill : public UKOGA_AttackBase
{
	GENERATED_BODY()
public:
    UKOGA_Charge_Skill();

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
    
    virtual void InputReleased(
       const FGameplayAbilitySpecHandle Handle,
       const FGameplayAbilityActorInfo* ActorInfo,
       const FGameplayAbilityActivationInfo ActivationInfo
    ) override;
    
    virtual void ExecuteAttack(float ChargeTime); 
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charge|Animation")
    UAnimMontage* ChargeMontage;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charge|Animation")
    FKOAttackMontageData AttackMontageData; 
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charge|Damage")
    UCurveFloat* ChargeDamageMultiplierCurve; 
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charge|Time")
    float MaxChargeTime;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Charge|Time")
    float ChargeTickRate;

private:
    FTimerHandle ChargeTimerHandle;
    float CurrentChargeTime;
    bool bIsCharging;
    
    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> ChargeMontageTask;
	
    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> AttackMontageTask; 
    
    UFUNCTION()
    void OnChargeMontageInterrupted();
    
    UFUNCTION()
    void OnAttackMontageCompleted();
    
    UFUNCTION()
    void OnAttackMontageInterrupted();
    
    UFUNCTION()
    void OnChargeTick();

    void StartCharging();
    void StopCharging();
    
    UFUNCTION()
    void OnTraceStart(FGameplayEventData Payload);

    UFUNCTION()
    void OnTraceEnd(FGameplayEventData Payload);
};
