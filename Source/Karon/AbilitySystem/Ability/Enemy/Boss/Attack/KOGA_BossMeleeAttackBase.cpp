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
	
	UAbilityTask_WaitGameplayEvent* WaitHitEventTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this,
		KOGameplayTags::Event_SkillHit,
		nullptr,
		false,
		false
	);
	
	WaitHitEventTask->EventReceived.AddDynamic(this, &UKOGA_BossMeleeAttackBase::OnHitEventReceived);
	WaitHitEventTask->ReadyForActivation();
}

void UKOGA_BossMeleeAttackBase::OnHitEventReceived(FGameplayEventData EventData)
{
	AActor* TargetActor = const_cast<AActor*>(EventData.Target.Get());
	ApplyHitEffects(TargetActor);
}
