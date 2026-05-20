#include "KOGA_AttackBase.h"

#include "Abilities/Tasks/AbilityTask_PlayAnimAndWait.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEffectApplied.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UKOGA_AttackBase::UKOGA_AttackBase()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UKOGA_AttackBase::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ACharacter* AvatarCharacter = GetAvatarCharacter();
	if (!AvatarCharacter || !AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		AttackMontage,
		1.0f	// 나중 오버클럭을 도입 시 공격 속도 계수로 교체
	);
	
	MontageTask->OnCompleted.AddDynamic(this, &UKOGA_AttackBase::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UKOGA_AttackBase::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &UKOGA_AttackBase::OnMontageCancelled);
	
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		AttackEventTag,
		nullptr,
		false,
		false
	); 

	EventTask->EventReceived.AddDynamic(this, &UKOGA_AttackBase::OnGameplayEventReceived);
	
	MontageTask->ReadyForActivation();
	EventTask->ReadyForActivation();
}

void UKOGA_AttackBase::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_AttackBase::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_AttackBase::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_AttackBase::OnGameplayEventReceived(FGameplayEventData Payload)
{
	if (!DamageEffectClass)
	{
		return;
	}
	
	for (TSharedPtr<FGameplayAbilityTargetData> TargetData : Payload.TargetData.Data)
	{
		if (TargetData.IsValid())
		{
			TArray<TWeakObjectPtr<AActor>> TargetActors = TargetData->GetActors();
			for (TWeakObjectPtr<AActor> WeakTarget : TargetActors)
			{
				if (AActor* TargetActor = WeakTarget.Get())
				{
					ApplyEffectToTarget(TargetActor, DamageEffectClass, GetAbilityLevel());
				}
			}
		}
	}
}
