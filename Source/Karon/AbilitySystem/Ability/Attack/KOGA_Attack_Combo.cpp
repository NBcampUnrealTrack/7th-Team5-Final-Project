#include "KOGA_Attack_Combo.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Utility/Log/KOLogManager.h"

UKOGA_Attack_Combo::UKOGA_Attack_Combo()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Attack_Heavy));
}

void UKOGA_Attack_Combo::ActivateAbility(
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
	
	// 1. Hit Event Task 
	UAbilityTask_WaitGameplayEvent* HitTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Hit);
	
	HitTask->EventReceived.AddDynamic(this, &ThisClass::OnHitEventReceived);
	HitTask->ReadyForActivation();

	// 2. Combo Window Tasks 
	UAbilityTask_WaitGameplayEvent* ComboWindowOpenTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Combo_Window_Open);
	
	ComboWindowOpenTask->EventReceived.AddDynamic(this, &ThisClass::OnComboWindowOpened);
	ComboWindowOpenTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* ComboWindowCloseTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Combo_Window_Close);
	
	ComboWindowCloseTask->EventReceived.AddDynamic(this, &ThisClass::OnComboWindowClosed);
	ComboWindowCloseTask->ReadyForActivation();
	
	// 3. Combo Transition Task 
	UAbilityTask_WaitGameplayEvent* ComboTransitionTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Combo_Transition);
	
	ComboTransitionTask->EventReceived.AddDynamic(this, &ThisClass::OnReceiveTransition);
	ComboTransitionTask->ReadyForActivation();
	
	PlayComboMontage(); 
}

void UKOGA_Attack_Combo::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	ComboIndex = 0;
	bNextComboRequested = false;
	bComboWindowOpen = false;
	bIsTransitioning = false;
	CurrentMontageTask = nullptr;
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Attack_Combo::InputPressed(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	
	if (bComboWindowOpen)
	{
		KO_LOG(Combat, Warning, TEXT("InputPressed in ComboWindow"));
		
		bNextComboRequested = true; 
	}
}

void UKOGA_Attack_Combo::PlayComboMontage()
{
	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnCompleted.RemoveDynamic(this, &ThisClass::OnMontageEnded);
		CurrentMontageTask->OnInterrupted.RemoveDynamic(this, &ThisClass::OnMontageEnded);
		CurrentMontageTask->EndTask();
		CurrentMontageTask = nullptr;
	}
	
	if (!MontageData.IsValidIndex(ComboIndex))
	{
		KO_LOG(Combat, Error, TEXT("ComboIndex %d is out of MontageData range (%d)."),
			ComboIndex, MontageData.Num());
		
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	KO_LOG(Combat, Warning, TEXT("Current ComboIndex : %d"), ComboIndex);
	const FName TaskName = FName(*FString::Printf(TEXT("MontageTask_%d"), ComboIndex));
	
	 CurrentMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TaskName,
			MontageData[ComboIndex].Montage,
			MontageData[ComboIndex].PlayRate
		);
	
	CurrentMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageEnded);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageEnded);
	CurrentMontageTask->ReadyForActivation();
}

void UKOGA_Attack_Combo::OnMontageEnded()
{
	if (bIsTransitioning) return;
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,true,false);
}

void UKOGA_Attack_Combo::OnComboWindowOpened(FGameplayEventData Payload)
{
	KO_LOG(Combat, Warning, TEXT("Combo check window opened."));
	
	bComboWindowOpen = true;
}

void UKOGA_Attack_Combo::OnComboWindowClosed(FGameplayEventData Payload)
{
	KO_LOG(Combat, Warning, TEXT("Combo check window closed."));
	
	bComboWindowOpen = false;
}

void UKOGA_Attack_Combo::OnHitEventReceived(FGameplayEventData Payload)
{
	SendAttackEventsToTarget(&Payload);
	ApplyHitEffects(&Payload);
}

void UKOGA_Attack_Combo::OnReceiveTransition(FGameplayEventData Payload)
{
	if (!bNextComboRequested) return; 
	
	KO_LOG(Combat, Warning, TEXT("Combo Transition %d -> %d."), ComboIndex, ComboIndex+1);
	
	bNextComboRequested = false;
	bComboWindowOpen = false;
	bIsTransitioning = true;
	ComboIndex++; 
	
	PlayComboMontage(); 
	
	bIsTransitioning = false;
}

