#include "KOGA_GuardBreak.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"

UKOGA_GuardBreak::UKOGA_GuardBreak()
{
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = KOGameplayTags::Event_Guard_Break;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
	
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_Guard_Break);
}

void UKOGA_GuardBreak::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask = 
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, BreakMontage);
	
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();
}

void UKOGA_GuardBreak::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_GuardBreak::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
