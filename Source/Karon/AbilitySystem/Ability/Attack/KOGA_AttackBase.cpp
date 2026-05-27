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
	
	FGameplayTag BaseEventTag = FGameplayTag::RequestGameplayTag(FName("Event"));
	
	UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		BaseEventTag,
		nullptr,
		false,
		false
	); 

	EventTask->EventReceived.AddDynamic(this, &UKOGA_AttackBase::OnGameplayEventReceived);
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

void UKOGA_AttackBase::OnMontageBlendOut()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
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
