#include "KOGA_BossAttackBase.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"

UKOGA_BossAttackBase::UKOGA_BossAttackBase()
{
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Boss_Groggy);
}
 
void UKOGA_BossAttackBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	UGameplayAbility::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbilityCost(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!AttackMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = 
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		AttackMontage,
		MontageSpeed,
		NAME_None,
		false,
		1.0f
	);
	
	MontageTask->OnCompleted.AddDynamic(this, &UKOGA_BossAttackBase::OnMontageCompleted);
	MontageTask->OnCancelled.AddDynamic(this, &UKOGA_BossAttackBase::OnMontageCancelled);
	MontageTask->OnInterrupted.AddDynamic(this, &UKOGA_BossAttackBase::OnMontageCancelled);
 
	MontageTask->ReadyForActivation();
}

void UKOGA_BossAttackBase::OnMontageCompleted()
{
	ApplyCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
	
	if (PostAttackDelay > 0.f && GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			PostAttackDelayHandle,
			this,
			&UKOGA_BossAttackBase::OnPostAttackDelayFinished,
			PostAttackDelay,
			false
		);
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
}

void UKOGA_BossAttackBase::OnPostAttackDelayFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
 
void UKOGA_BossAttackBase::OnMontageCancelled()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(PostAttackDelayHandle);
	}
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
