#include "KOGA_Guard.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Utility/Log/KOLogManager.h"

UKOGA_Guard::UKOGA_Guard()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; 
	
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Combat_Guard));
	ActivationRequiredTags.AddTag(KOGameplayTags::State_Character_WeaponDrawn);
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_Guard);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Guard_Break);
}

void UKOGA_Guard::ActivateAbility(
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
	
	UAbilityTask_WaitGameplayEvent* GuardFailEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Guard_DirectionFail);
	
	GuardFailEventTask->EventReceived.AddDynamic(this, &ThisClass::OnGuardStart);
	GuardFailEventTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* GuardStartEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Guard_Start);
	
	GuardStartEventTask->EventReceived.AddDynamic(this, &ThisClass::OnGuardStart);
	GuardStartEventTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* GuardEndEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Guard_End);
	
	GuardEndEventTask->EventReceived.AddDynamic(this, &ThisClass::OnGuardEnd);
	GuardEndEventTask->ReadyForActivation(); 
	
	UAbilityTask_WaitGameplayEvent* PerfectGuardStartEvent =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_PerfectGuard_Start);
	
	PerfectGuardStartEvent->EventReceived.AddDynamic(this, &ThisClass::OnPerfectWindowStart);
	PerfectGuardStartEvent->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* PerfectGuardEndEvent =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_PerfectGuard_End);
	
	PerfectGuardEndEvent->EventReceived.AddDynamic(this, &ThisClass::OnPerfectWindowEnd);
	PerfectGuardEndEvent->ReadyForActivation();
	
	UAbilityTask_PlayMontageAndWait* MontageTask = 
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, GuardMontage);
	
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation(); 
}

void UKOGA_Guard::InputReleased(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
	
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		if (ASC->GetCurrentMontage() == GuardMontage)
		{
			ASC->CurrentMontageJumpToSection(TEXT("End"));
			
			return;
		}
	}
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UKOGA_Guard::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, 
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UAbilitySystemComponent* ASC = GetASC())
	{
		ASC->SetLooseGameplayTagCount(KOGameplayTags::State_Character_Guard_PerfectGuard, 0); 
		ASC->SetLooseGameplayTagCount(KOGameplayTags::State_Character_Guard_Blocking, 0); 
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Guard::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UKOGA_Guard::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Guard::OnGuardFailed(FGameplayEventData Data)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;
	
	ASC->RemoveLooseGameplayTag(KOGameplayTags::State_Character_Guard_Blocking);
	if (GE_Guard_Reset) ApplyEffectToSelf(GE_Guard_Reset); 
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Guard::OnGuardStart(FGameplayEventData Data)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;
	
	ASC->AddLooseGameplayTag(KOGameplayTags::State_Character_Guard_Blocking);
	if (GE_Guard_Init) ApplyEffectToSelf(GE_Guard_Init); 
	
	ASC->RemoveLooseGameplayTag(KOGameplayTags::State_Character_Guard_PerfectGuard);
}

void UKOGA_Guard::OnGuardEnd(FGameplayEventData Data)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;
	
	ASC->RemoveLooseGameplayTag(KOGameplayTags::State_Character_Guard_Blocking);
	if (GE_Guard_Reset) ApplyEffectToSelf(GE_Guard_Reset); 
}

void UKOGA_Guard::OnPerfectWindowStart(FGameplayEventData Data)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;
	
	ASC->AddLooseGameplayTag(KOGameplayTags::State_Character_Guard_PerfectGuard);
}

void UKOGA_Guard::OnPerfectWindowEnd(FGameplayEventData Data)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;
	
	ASC->RemoveLooseGameplayTag(KOGameplayTags::State_Character_Guard_PerfectGuard);
}
