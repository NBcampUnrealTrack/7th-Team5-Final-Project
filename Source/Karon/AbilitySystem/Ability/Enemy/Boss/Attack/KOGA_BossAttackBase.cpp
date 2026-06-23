#include "KOGA_BossAttackBase.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"

#include "Character/Enemy/Boss/KOBossBase.h"

UKOGA_BossAttackBase::UKOGA_BossAttackBase()
{
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_Attacking);
	
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Boss_Groggy);
}
 
void UKOGA_BossAttackBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// Super에서 EndAbility 호출됐는지 체크
	if (!IsActive())
	{
		return;
	}
	
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
 
	// 몽타주 재생 태스크 생성
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
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
 
void UKOGA_BossAttackBase::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
