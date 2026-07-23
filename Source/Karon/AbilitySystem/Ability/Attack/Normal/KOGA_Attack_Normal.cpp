#include "KOGA_Attack_Normal.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_Tick.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"

UKOGA_Attack_Normal::UKOGA_Attack_Normal()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UKOGA_Attack_Normal::ActivateAbility(
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
	
	const UKOCombatSet* CombatSet = GetCombatSet();
	if (MontageData.IsEmpty() || !CombatSet)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_WaitGameplayEvent* SocketSwapTask  = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_Start);
	
	SocketSwapTask->EventReceived.AddDynamic(this, &UKOGA_Attack_Normal::OnSocketSwap);
	SocketSwapTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* TraceStartTask  = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_Start);
	
	TraceStartTask->EventReceived.AddDynamic(this, &UKOGA_Attack_Normal::OnTraceStart);
	TraceStartTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* TraceEndTask =
	  UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_End);
	
	TraceEndTask->EventReceived.AddDynamic(this, &ThisClass::OnTraceEnd);
	TraceEndTask->ReadyForActivation();
	

	float PlayRate = MontageData[0].PlayRate * CombatSet->GetAttackSpeed();

	UAbilityTask_PlayMontageAndWait* PlayMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, MontageData[0].Montage, PlayRate);
	
	PlayMontageTask->OnCompleted.AddDynamic(this, &UKOGA_Attack_Normal::OnMontageCompleted);
	PlayMontageTask->OnCancelled.AddDynamic(this, &UKOGA_Attack_Normal::OnMontageCancelled);
	PlayMontageTask->OnInterrupted.AddDynamic(this, &UKOGA_Attack_Normal::OnMontageCancelled);
	PlayMontageTask->ReadyForActivation();
}

void UKOGA_Attack_Normal::OnSocketSwap(FGameplayEventData EventData)
{
	TraceData.SwapSocket();
}

void UKOGA_Attack_Normal::OnTraceStart(FGameplayEventData EventData)
{
	ResetHitActors();
	
	TickTask = UAbilityTask_Tick::CreateTickTask(this);
	TickTask->OnTick.AddDynamic(this, &ThisClass::PerformWeaponTrace);
	TickTask->ReadyForActivation();
}

void UKOGA_Attack_Normal::OnTraceEnd(FGameplayEventData EventData)
{
	if (TickTask)
	{
		TickTask->StopTask();
		TickTask = nullptr;
	}
}

void UKOGA_Attack_Normal::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Attack_Normal::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}


