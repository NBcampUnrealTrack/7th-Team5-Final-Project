#include "KOGA_HitReaction.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"

UKOGA_HitReaction::UKOGA_HitReaction()
{
	InstancingPolicy  = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = KOGameplayTags::Event_HitReact; 
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
	
	
}

void UKOGA_HitReaction::ActivateAbility(
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
}

void UKOGA_HitReaction::EndAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, 
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_HitReaction::OnHitStopFinished()
{
}

void UKOGA_HitReaction::OnMontageCompleted()
{
}

void UKOGA_HitReaction::OnMontageCancelled()
{
}
