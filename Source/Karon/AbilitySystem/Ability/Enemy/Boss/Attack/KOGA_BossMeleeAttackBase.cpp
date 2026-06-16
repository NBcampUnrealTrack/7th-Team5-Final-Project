#include "AbilitySystem/Ability/Enemy/Boss/Attack/KOGA_BossMeleeAttackBase.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

UKOGA_BossMeleeAttackBase::UKOGA_BossMeleeAttackBase()
{
}

void UKOGA_BossMeleeAttackBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
 
	if (!IsActive())
	{
		return;
	}
	
	// 히트 이벤트 대기 (NotifyState에서 발생)
	WaitHitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		KOGameplayTags::Event_SkillHit,
		nullptr,
		false,
		false
	);
 
	if (WaitHitEventTask)
	{
		WaitHitEventTask->EventReceived.AddDynamic(this, &UKOGA_BossMeleeAttackBase::OnHitEventReceived);
		WaitHitEventTask->ReadyForActivation();
	}
}
 
void UKOGA_BossMeleeAttackBase::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	if (WaitHitEventTask)
	{
		WaitHitEventTask->EndTask();
		WaitHitEventTask = nullptr;
	}
 
	Super::EndAbility(Handle, ActorInfo, ActivationInfo,
		bReplicateEndAbility, bWasCancelled);
}

void UKOGA_BossMeleeAttackBase::OnHitEventReceived(FGameplayEventData EventData)
{
	if (!IsActive())
	{
		return;
	}
	
	AActor* TargetActor = const_cast<AActor*>(EventData.Target.Get());
	ApplyDamageToTarget(TargetActor);
}
