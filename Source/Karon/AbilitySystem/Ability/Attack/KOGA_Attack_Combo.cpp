#include "KOGA_Attack_Combo.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_Tick.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "GameFramework/Character.h"
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
	
	// 1. Trace Event Task
	UAbilityTask_WaitGameplayEvent* TraceStartTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_Start);
	
	TraceStartTask->EventReceived.AddDynamic(this, &ThisClass::OnTraceStart);
	TraceStartTask->ReadyForActivation(); 

	UAbilityTask_WaitGameplayEvent* TraceEndTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Trace_End);
	
	TraceEndTask->EventReceived.AddDynamic(this, &ThisClass::OnTraceEnd);
	TraceEndTask->ReadyForActivation(); 
	
	// 2. Hit Event Task
	UAbilityTask_WaitGameplayEvent* HitTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Hit);
	
	HitTask->EventReceived.AddDynamic(this, &ThisClass::OnHitEventReceived);
	HitTask->ReadyForActivation();

	// 3. Combo Window Tasks 
	UAbilityTask_WaitGameplayEvent* ComboWindowOpenTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Combo_Window_Open);
	
	ComboWindowOpenTask->EventReceived.AddDynamic(this, &ThisClass::OnComboWindowOpened);
	ComboWindowOpenTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* ComboWindowCloseTask = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Combo_Window_Close);
	
	ComboWindowCloseTask->EventReceived.AddDynamic(this, &ThisClass::OnComboWindowClosed);
	ComboWindowCloseTask->ReadyForActivation();
	
	// 4. Combo Transition Task 
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
	
	if (CurrentMontageTask)
	{
		CurrentMontageTask->EndTask();
		CurrentMontageTask = nullptr;
	}
	
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

void UKOGA_Attack_Combo::OnTargetHit(const FHitResult& Hit)
{
	Super::OnTargetHit(Hit);
	
	UAbilitySystemComponent* SourceASC = GetASC();
	if (SourceASC && OverClockGainEffectClass)
	{
		if (!SourceASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_OverClock))
		{
			FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
			Context.AddSourceObject(GetAvatarCharacter());
			
			ApplyEffectToSelf(OverClockGainEffectClass, 1.f);
			SourceASC->ApplyGameplayEffectToSelf(OverClockGainEffectClass->GetDefaultObject<UGameplayEffect>(), 1.0f, Context);
			UE_LOG(LogTemp, Warning, TEXT("[Overclock] 타격 성공 게이지 상승 이펙트 적용 타겟: %s"), *Hit.GetActor()->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Overclock] 이미 오버클럭 상태임으로 게이지 획득 스킵"));
		}
	}
}

void UKOGA_Attack_Combo::PlayComboMontage()
{
	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnCompleted.RemoveDynamic(this, &ThisClass::OnMontageCompleted);
		CurrentMontageTask->OnInterrupted.RemoveDynamic(this, &ThisClass::OnMontageCompleted);
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
	
	float PlayRate = MontageData[ComboIndex].PlayRate;
	if (UKOCombatSet* CombatSet = GetCombatSet())
	{
		PlayRate *= CombatSet->GetAttackSpeed();
	}
	
	 CurrentMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TaskName,
			MontageData[ComboIndex].Montage,
			PlayRate
		);
	
	ResetHitActors();
	
	CurrentMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	CurrentMontageTask->ReadyForActivation();
}

void UKOGA_Attack_Combo::OnMontageCompleted()
{
	if (bIsTransitioning) return;
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,true,false);
}

void UKOGA_Attack_Combo::OnMontageInterrupted()
{
	bIsTransitioning = false;
    
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Attack_Combo::OnTraceStart(FGameplayEventData Payload)
{
	ResetHitActors();
	
	TickTask = UAbilityTask_Tick::CreateTickTask(this);
	TickTask->OnTick.AddDynamic(this, &ThisClass::PerformWeaponTrace);
	TickTask->ReadyForActivation();
}

void UKOGA_Attack_Combo::OnTraceEnd(FGameplayEventData Payload)
{
	if (TickTask)
	{
		TickTask->StopTask();
		TickTask = nullptr;
	}
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

