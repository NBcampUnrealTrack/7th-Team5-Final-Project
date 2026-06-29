#include "KOGA_Gurad.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"

UKOGA_Gurad::UKOGA_Gurad()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; 
}

void UKOGA_Gurad::ActivateAbility(
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
	
	UAbilityTask_WaitGameplayEvent* WaitHitEvent = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, KOGameplayTags::Event_HitReact, nullptr, true);
	
	WaitHitEvent->EventReceived.AddDynamic(this, &ThisClass::OnHitReceived);
	WaitHitEvent->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* PerfectGuardStartEvent =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Dodge_PerfectGuard_Start);
	
	PerfectGuardStartEvent->EventReceived.AddDynamic(this, &ThisClass::OnPerfectWindowStart);
	PerfectGuardStartEvent->ReadyForActivation(); 
	
	UAbilityTask_WaitGameplayEvent* PerfectGuardEndEvent =
	UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Dodge_PerfectGuard_End);
	
	PerfectGuardEndEvent->EventReceived.AddDynamic(this, &ThisClass::OnPerfectWindowEnd);
	PerfectGuardEndEvent->ReadyForActivation(); 
	
	UAbilityTask_PlayMontageAndWait* MontageTask = 
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, GuardMontage);
	
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation(); 
}

void UKOGA_Gurad::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Gurad::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, 
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Gurad::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Gurad::OnPerfectWindowStart(FGameplayEventData Data)
{
	bPerfectGuardWindowOpen = true;
}

void UKOGA_Gurad::OnPerfectWindowEnd(FGameplayEventData Data)
{
	bPerfectGuardWindowOpen = false;
}

void UKOGA_Gurad::OnHitReceived(FGameplayEventData Data)
{
	if (bPerfectGuardWindowOpen) // 패링 성공 
	{
		
	}
	else // 패링 실패 
	{
		
	}
}
