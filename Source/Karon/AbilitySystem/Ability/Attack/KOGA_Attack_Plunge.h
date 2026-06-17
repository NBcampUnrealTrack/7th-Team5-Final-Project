// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/Attack/KOGA_AttackBase.h"
#include "NativeGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
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
		const FGameplayEventData* TriggerEventData
		)override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
		) override;
	
	UFUNCTION()
	virtual void OnGameplayEventReceived(FGameplayEventData Payload);
	
	//애니메이션
	UPROPERTY(EditDefaultsOnly, Category="Plunge|Animation")
	TObjectPtr<UAnimMontage> ChargeMontage;   // 차징 중 재생, 버튼 누르는 순간

	UPROPERTY(EditDefaultsOnly, Category="Plunge|Animation")
	TObjectPtr<UAnimMontage> FallMontage;     // 하강 중 재생, 버튼 떼고 하강 중

	UPROPERTY(EditDefaultsOnly, Category="Plunge|Animation")
	TObjectPtr<UAnimMontage> LandMontage;     // 착지 시 재생

	// ─── 차징 설정 ───────────────────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category="Plunge|Charge")
	float MaxChargeTime = 2.0f;              // 최대 차징 시간 (초)

	UPROPERTY(EditDefaultsOnly, Category="Plunge|Charge")
	float MaxDamageMultiplier = 3.0f;        // 풀차징 시 데미지 배율

	UPROPERTY(EditDefaultsOnly, Category="Plunge|Charge")
	float PlungeSpeed = 2000.0f;             // 하강 속도 (cm/s)

private:
	// 내부 상태
	float ChargeStartTime = 0.f;
	float ChargeRatio = 0.f;                 // 0.0 ~ 1.0
	bool bIsPlunging = false;

	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> CurrentMontageTask;

	// 페이즈별 함수
	void StartCharge();
	void StartPlunge();
	void StartLanding();

	// 콜백
	UFUNCTION() void OnChargeMontageCompleted();
	UFUNCTION() void OnFallMontageCompleted();
	UFUNCTION() void OnLandMontageCompleted();
	void OnMontageCancelled();
	UFUNCTION() void OnInputReleased(float TimeHeld); // 버튼 뗐을 때
};
