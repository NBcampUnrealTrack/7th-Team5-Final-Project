#include "KOGA_Charge_Attack_Base.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

UKOGA_Charge_Attack_Base::UKOGA_Charge_Attack_Base()
{
	MaxChargeTime = 2.0f;
	ChargeTickRate = 0.05f;
	CurrentChargeTime = 0.0f;
	bIsCharging = false;
}

void UKOGA_Charge_Attack_Base::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (ChargeMontage)
	{
		ChargeMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ChargeMontage,
			1.0f,
			NAME_None,
			false
		);
		
		if (ChargeMontageTask)
		{
			ChargeMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::UKOGA_Charge_Attack_Base::OnChargeMontageInterrupted);
			ChargeMontageTask->OnCancelled.AddDynamic(this, &ThisClass::UKOGA_Charge_Attack_Base::OnChargeMontageInterrupted);
			ChargeMontageTask->ReadyForActivation();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ChargeMontage가 없음"), *GetName());
	}
	
	StartCharging();
	
	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this);
	if (ReleaseTask)
	{
		ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
		ReleaseTask->ReadyForActivation();
	}
}

void UKOGA_Charge_Attack_Base::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	StopCharging();
}

void UKOGA_Charge_Attack_Base::ExecuteAttack(float ChargePercentage)
{
	// TODO:  
	// 예: 최소 데미지는 보장하되(30%), 차징 비율에 따라 최대 100%까지 증가
	// float DamageMultiplier = 0.3f + (ChargePercentage * 0.7f);
}

void UKOGA_Charge_Attack_Base::OnInputReleased(float TimeHeld)
{
	if (bIsCharging)
	{
		StopCharging();
		
		float ChargePercentage = FMath::Clamp(CurrentChargeTime / MaxChargeTime, 0.0f, 1.0f);
		UE_LOG(LogTemp, Warning, TEXT("입력 해제. 차징 비율: %f (%.1f%%)"), ChargePercentage, ChargePercentage * 100.0f);
		
		ExecuteAttack(ChargePercentage);
	}
}

void UKOGA_Charge_Attack_Base::OnChargeMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Charge_Attack_Base::OnChargeTick()
{
	if (!bIsCharging)
	{
		return;
	}
	
	CurrentChargeTime += ChargeTickRate;
	
	if (CurrentChargeTime >= MaxChargeTime)
	{
		StopCharging();
		ExecuteAttack(1.0f);
	}
}

void UKOGA_Charge_Attack_Base::StartCharging()
{
	CurrentChargeTime = 0.0f;
	bIsCharging = true;
	
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			ChargeTimerHandle,
			this,
			&ThisClass::OnChargeTick,
			ChargeTickRate,
			true
		);
	}
}

void UKOGA_Charge_Attack_Base::StopCharging()
{
	bIsCharging = false;
	
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(ChargeTimerHandle);
	}
	
	if (ChargeMontageTask)
	{
		ChargeMontageTask->EndTask();
		ChargeMontageTask = nullptr;
	}
}
