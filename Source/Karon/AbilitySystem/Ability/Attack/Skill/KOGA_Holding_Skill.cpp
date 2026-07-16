#include "KOGA_Holding_Skill.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_Tick.h"
#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Data/KO_HitData.h"

UKOGA_Holding_Skill::UKOGA_Holding_Skill()
{
	MaxHoldTime = 3.0f;
	bIsHolding = false;
	bIsFinisherActive = false;
}

void UKOGA_Holding_Skill::ActivateAbility(
	FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	DamageEffects = HoldDamageEffect;
	AdditionalEffects = AdditionalHoldDamageEffect;
	
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!IsActive()) return;
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!HoldMontageData.Montage || !FinisherMontageData.Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	bIsHolding = true;
	bIsFinisherActive = false;
	
	UAbilityTask_WaitGameplayEvent* TraceStartTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_Start);
	if (TraceStartTask)
	{
		TraceStartTask->EventReceived.AddDynamic(this, &ThisClass::OnTraceStart);
		TraceStartTask->ReadyForActivation();
	}
	
	UAbilityTask_WaitGameplayEvent* TraceEndTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_End);
	if (TraceEndTask)
	{
		TraceEndTask->EventReceived.AddDynamic(this, &ThisClass::OnTraceStart);
		TraceEndTask->ReadyForActivation();
	}
	
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer(
			MaxHoldTimerHandle, this, &ThisClass::ExecuteFinisher, MaxHoldTime, false
		);
	}
	
	HoldMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, HoldMontageData.Montage, HoldMontageData.PlayRate
	);
    
	if (HoldMontageTask)
	{
		HoldMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnHoldMontageInterrupted);
		HoldMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnHoldMontageInterrupted);
		HoldMontageTask->ReadyForActivation();
	}
}

void UKOGA_Holding_Skill::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(MaxHoldTimerHandle);
	}
    
	if (TickTask)
	{
		TickTask->StopTask();
		TickTask = nullptr;
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Holding_Skill::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (bIsHolding && !bIsFinisherActive)
	{
		ExecuteFinisher();
	}
}

void UKOGA_Holding_Skill::ExecuteFinisher()
{
	if (bIsFinisherActive) return;

	bIsHolding = false;
	bIsFinisherActive = true;

	DamageEffects = FinishDamageEffect;
	AdditionalEffects = AdditionalFinishDamageEffect;
	
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(MaxHoldTimerHandle);
	}
    
	if (HoldMontageTask)
	{
		HoldMontageTask->EndTask();
		HoldMontageTask = nullptr;
	}
    
	if (TickTask)
	{
		TickTask->StopTask();
		TickTask = nullptr;
	}
	ResetHitActors();

	FinisherMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, FinisherMontageData.Montage, FinisherMontageData.PlayRate
	);

	if (FinisherMontageTask)
	{
		FinisherMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnFinisherMontageCompleted);
		FinisherMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnFinisherMontageInterrupted);
		FinisherMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnFinisherMontageInterrupted);
		FinisherMontageTask->ReadyForActivation();
	}
}

void UKOGA_Holding_Skill::OnHoldMontageInterrupted()
{
	if (!bIsFinisherActive)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

void UKOGA_Holding_Skill::OnFinisherMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Holding_Skill::OnFinisherMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Holding_Skill::OnTraceStart(FGameplayEventData Payload)
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

void UKOGA_Holding_Skill::OnTraceEnd(FGameplayEventData Payload)
{
	if (TickTask)
	{
		TickTask->StopTask();
		TickTask = nullptr;
	}
}
