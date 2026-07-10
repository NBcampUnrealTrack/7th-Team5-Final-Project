#include "KOGA_Normal_Skill.h"
#include "AbilitySystemComponent.h" 
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_Tick.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "AbilitySystem/Tag/Input/KOGameplayTags_Input.h"
#include "Data/KO_HitData.h"

UKOGA_Normal_Skill::UKOGA_Normal_Skill()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Skill_DiveSlash));
	
}

void UKOGA_Normal_Skill::ActivateAbility(
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
	
	if (MontageData.IsEmpty() || MontageData[0].Montage == nullptr)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilityTask_WaitGameplayEvent* TraceStartTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, 
		KOGameplayTags::Event_Trace_Start
	);
	if (TraceStartTask)
	{
		TraceStartTask->EventReceived.AddDynamic(this, &ThisClass::OnTraceStart);
		TraceStartTask->ReadyForActivation(); 
	}

	UAbilityTask_WaitGameplayEvent* TraceEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, 
		KOGameplayTags::Event_Trace_End
	);
	if (TraceEndTask)
	{
		TraceEndTask->EventReceived.AddDynamic(this, &ThisClass::OnTraceEnd);
		TraceEndTask->ReadyForActivation(); 
	}
	
	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		MontageData[0].Montage,
		MontageData[0].PlayRate
	);	
	
	if (PlayMontageTask)
	{
		PlayMontageTask->OnCompleted.AddDynamic(this, &UKOGA_Normal_Skill::OnMontageCompleted);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &UKOGA_Normal_Skill::OnMontageCancelled);
		PlayMontageTask->OnCancelled.AddDynamic(this, &UKOGA_Normal_Skill::OnMontageCancelled);
		PlayMontageTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UKOGA_Normal_Skill::OnTraceStart(FGameplayEventData Payload)
{
	if (const UKO_HitData* ReceivedData = Cast<UKO_HitData>(Payload.OptionalObject))
	{
		CachedHitData = ReceivedData;
		HitStopDuration = ReceivedData->HitData.HitStopDuration;
		HitStopTimeDilation = ReceivedData->HitData.HitStopTimeDilation;
	}

	ResetHitActors();
	TraceData.bIsFirstTick = true;
    
	TickTask = UAbilityTask_Tick::CreateTickTask(this);
	if (TickTask)
	{
		TickTask->OnTick.AddDynamic(this, &ThisClass::PerformWeaponTrace);
		TickTask->ReadyForActivation();
	}
}
    
void UKOGA_Normal_Skill::OnTraceEnd(FGameplayEventData Payload)
{
	if (TickTask)
	{
		TickTask->StopTask();
		TickTask = nullptr;
	}
}

void UKOGA_Normal_Skill::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Normal_Skill::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
