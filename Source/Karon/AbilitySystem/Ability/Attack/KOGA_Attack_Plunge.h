#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/Attack/KOGA_AttackBase.h"
#include "NativeGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "GameplayEffect.h"
#include "KOGA_Attack_Plunge.generated.h"

UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Input_Ability_Attack_Plunge);
UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Event_Plunge_Land);

UCLASS()
class KARON_API UKOGA_Attack_Plunge : public UKOGA_AttackBase
{
    GENERATED_BODY()

public:
    UKOGA_Attack_Plunge();

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void InputReleased(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo) override;
    
    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;

    
    

    // BP에서 설정: 칼 장착 상태 태그 (비워두면 항상 발동)
    UPROPERTY(EditDefaultsOnly, Category = "Plunge|Condition")
    FGameplayTag RequiredWeaponTag;

    // ChargeMontage는 부모에서 상속
    UPROPERTY(EditDefaultsOnly, Category = "Plunge|Animation")
    TObjectPtr<UAnimMontage> ChargeMontage;
    
    UPROPERTY(EditDefaultsOnly, Category = "Plunge|Animation")
    TObjectPtr<UAnimMontage> FallMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Plunge|Animation")
    TObjectPtr<UAnimMontage> LandMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Plunge|Charge")
    float PlungeSpeed = 2000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Plunge|Charge")
    float MaxChargeTime = 2.0f; // 최대 차징 시간
    
    UPROPERTY(EditDefaultsOnly, Category = "Plunge|Charge")
    float MaxDamageMultiplier = 3.0f;
    
    

private:
    float ChargeStartTime = 0.f;
    float ChargeRatio = 0.f;
    bool  bIsPlunging = false;

    UPROPERTY()
    TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;
    
    void StartPlunge();
    void StartLanding();

    UFUNCTION() void OnFallMontageCompleted();
    UFUNCTION() void OnLandMontageCompleted();
    UFUNCTION() void OnMontageCancelled();
    UFUNCTION() void OnLandEventReceived(FGameplayEventData Payload);
};
