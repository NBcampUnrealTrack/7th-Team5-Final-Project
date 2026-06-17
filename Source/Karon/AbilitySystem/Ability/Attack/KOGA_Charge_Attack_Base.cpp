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
			true
		);
		
		if (ChargeMontageTask)
		{
			ChargeMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::UKOGA_Charge_Attack_Base::OnChargeMontageInterrupted);
			ChargeMontageTask->OnCancelled.AddDynamic(this, &ThisClass::UKOGA_Charge_Attack_Base::OnChargeMontageInterrupted);
			ChargeMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnChargeMontageInterrupted);
			ChargeMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnChargeMontageInterrupted);
			ChargeMontageTask->ReadyForActivation();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s] ChargeMontage가 없음"), *GetName());
	}
	
	StartCharging();

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
	// 추후 자식 클래스 만들 때 아래의 EndAbility는 지우고 자식클래스에서 불러오면 됨.
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Charge_Attack_Base::InputReleased(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo)
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
		CurrentChargeTime = MaxChargeTime;
		
		UWorld* World = GetWorld();
		if (World)
		{
			World->GetTimerManager().ClearTimer(ChargeTimerHandle);
		}
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
