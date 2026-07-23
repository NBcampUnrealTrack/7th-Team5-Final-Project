#include "KOGA_Attack_Combo.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Ability/AbilityTask/AbilityTask_Tick.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "GameFramework/Character.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Utility/FunctionLibrary/FunctionLibrary.h"
#include "Utility/Log/KOLogManager.h"

UKOGA_Attack_Combo::UKOGA_Attack_Combo()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Attack_Heavy));
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Movement_InAir);
}

void UKOGA_Attack_Combo::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//근처에 그로기된 적이 있다면 패리어택 공격 실행
	ACharacter* Character = GetAvatarCharacter();
	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(Character);
	TArray<TWeakObjectPtr<AActor>> DetectedActors;
	UFunctionLibrary::FindActorsWithGameplayTagInRange(
		GetWorld(),
		Character->GetActorLocation(),
		GroggyRadius,
		KOGameplayTags::State_Enemy_Parried,
		IgnoredActors,
		DetectedActors
		);
	
	bool bCanGroggyAttack=false;
	
	//그로기된 적이 있는 경우 어빌리티 즉시종료 및 패리어택 어빌리티 실행
	if (DetectedActors.Num() != 0)
	{
		//첫번째 액터가 그로기 루프 중인지 체크
		if (AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(DetectedActors[0]))
		{
			if (UAnimInstance* AnimInstance = Enemy->GetMesh()? Enemy->GetMesh()->GetAnimInstance():nullptr)
			{
				//현재 실행중인 몽타주 섹션 이름을 반환
				FName CurrentSectionName = AnimInstance->Montage_GetCurrentSection();
				if (CurrentSectionName == GroggyLoopName)
				{
					bCanGroggyAttack=true;
				}
			}
		}
		
		if (bCanGroggyAttack)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		
			//다음 어빌리티에 감지된 데이터 전달
			FGameplayAbilityTargetData_ActorArray* NewData = new FGameplayAbilityTargetData_ActorArray();
			FGameplayAbilityTargetDataHandle TargetDataHandle;
			NewData->TargetActorArray = DetectedActors;
			TargetDataHandle.Add(NewData);
		
			//게임플레이 이벤트 데이터에 값 주입
			FGameplayEventData EventData;
			EventData.EventTag = KOGameplayTags::Event_ParryAttack;
			EventData.TargetData = TargetDataHandle; 

			// 게임플레이 이벤트를 통해 어빌리티 발동
			if (UAbilitySystemComponent* ASC = GetASC())
			{
				ASC->HandleGameplayEvent(EventData.EventTag, &EventData);
			}
			return;
		}
	}
	
	
	
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
	CurrentMontageIndex = 0;
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

void UKOGA_Attack_Combo::PlayComboMontage()
{
	if (CurrentMontageTask)
	{
		CurrentMontageTask->OnCompleted.RemoveDynamic(this, &ThisClass::OnMontageCompleted);
		CurrentMontageTask->OnInterrupted.RemoveDynamic(this, &ThisClass::OnMontageCancelled);
		CurrentMontageTask->OnCancelled.RemoveDynamic(this, &ThisClass::OnMontageCancelled);
		CurrentMontageTask->EndTask();
		CurrentMontageTask = nullptr;
	}
	
	if (!MontageData.IsValidIndex(CurrentMontageIndex))
	{
		KO_LOG(Combat, Error, TEXT("ComboIndex %d is out of MontageData range (%d)."),
			CurrentMontageIndex, MontageData.Num());
		
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	if (MotionWarpData.bUseMotionWarping)
	{
		UpdateMotionWarpTarget();
	}
	
	KO_LOG(Combat, Warning, TEXT("Current ComboIndex : %d"), CurrentMontageIndex);
	const FName TaskName = FName(*FString::Printf(TEXT("MontageTask_%d"), CurrentMontageIndex));
	
	float PlayRate = MontageData[CurrentMontageIndex].PlayRate;
	if (UKOCombatSet* CombatSet = GetCombatSet())
	{
		PlayRate *= CombatSet->GetAttackSpeed();
	}
	
	 CurrentMontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, TaskName,
			MontageData[CurrentMontageIndex].Montage,
			PlayRate
		);
	
	ResetHitActors();
	
	CurrentMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	CurrentMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	CurrentMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	CurrentMontageTask->ReadyForActivation();
}

void UKOGA_Attack_Combo::OnMontageCompleted()
{
	if (bIsTransitioning) return;
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,true,false);
}

void UKOGA_Attack_Combo::OnMontageCancelled()
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
	
	KO_LOG(Combat, Warning, TEXT("Combo Transition %d -> %d."), CurrentMontageIndex, CurrentMontageIndex+1);
	
	bNextComboRequested = false;
	bComboWindowOpen = false;
	bIsTransitioning = true;
	CurrentMontageIndex++; 
	
	PlayComboMontage();
	
	bIsTransitioning = false;
}

