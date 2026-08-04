#include "KOGA_Guard.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitAttributeChange.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Attribute/KOStaminaSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Utility/Log/KOLogManager.h"

UKOGA_Guard::UKOGA_Guard()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor; 
	
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Combat_Guard));
	
	ActivationRequiredTags.AddTag(KOGameplayTags::State_Character_WeaponDrawn);
	
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_Guard);
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_NoStaminaRegen);
	
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Guard_Break);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_HitReacting);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_StaminaExhausted);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Movement_InAir);
}

bool UKOGA_Guard::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) 
		return false; 
	
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return false;
	 
	const float Stamina = ASC->GetNumericAttribute(UKOStaminaSet::GetStaminaAttribute());
	if (Stamina <= 5.f) return false;
	
	return true; 
}

void UKOGA_Guard::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbilityCost(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}
	
	// 1. Check Stamina Task 
	UAbilityTask_WaitAttributeChange* CheckStaminaTask =
		UAbilityTask_WaitAttributeChange::WaitForAttributeChange(
			this,
			UKOStaminaSet::GetStaminaAttribute(),
			FGameplayTag::EmptyTag,
			FGameplayTag::EmptyTag,
			false
		);
	
	CheckStaminaTask->OnChange.AddDynamic(this, &ThisClass::OnStaminaChanged);
	CheckStaminaTask->ReadyForActivation(); 
	
	// 2. Guard Result Task (Success / Fail) 
	UAbilityTask_WaitGameplayEvent* GuardSuccessEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Guard_Success);
	
	GuardSuccessEventTask->EventReceived.AddDynamic(this, &ThisClass::OnGuardSuccess);
	GuardSuccessEventTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* GuardFailEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Guard_DirectionFail);
	
	GuardFailEventTask->EventReceived.AddDynamic(this, &ThisClass::OnGuardFailed);
	GuardFailEventTask->ReadyForActivation();
	
	// 3. Guard Start/End Task 
	UAbilityTask_WaitGameplayEvent* GuardStartEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Guard_Start);
	
	GuardStartEventTask->EventReceived.AddDynamic(this, &ThisClass::OnGuardStart);
	GuardStartEventTask->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* GuardEndEventTask =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Guard_End);
	
	GuardEndEventTask->EventReceived.AddDynamic(this, &ThisClass::OnGuardEnd);
	GuardEndEventTask->ReadyForActivation(); 
	
	// 4. Perfect Guard Start/End Task 
	UAbilityTask_WaitGameplayEvent* PerfectGuardStartEvent =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_PerfectGuard_Start);
	
	PerfectGuardStartEvent->EventReceived.AddDynamic(this, &ThisClass::OnPerfectWindowStart);
	PerfectGuardStartEvent->ReadyForActivation();
	
	UAbilityTask_WaitGameplayEvent* PerfectGuardEndEvent =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_PerfectGuard_End);
	
	PerfectGuardEndEvent->EventReceived.AddDynamic(this, &ThisClass::OnPerfectWindowEnd);
	PerfectGuardEndEvent->ReadyForActivation();
	
	// 5. Montage Task
	 UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, GuardMontage);

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation();

	// 6. Watchdog
	ActivationTime = GetWorld()->GetTimeSeconds();
	bWatchdogLogged = false;
	WatchdogTask = UAbilityTask_Tick::CreateTickTask(this);
	WatchdogTask->OnTick.AddDynamic(this, &ThisClass::CheckGuardLifetime);
	WatchdogTask->ReadyForActivation();
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
	
	CommitAbilityCooldown(Handle, ActorInfo, ActivationInfo, true);
	
	EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
}

void UKOGA_Guard::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (WatchdogTask)
	{
		WatchdogTask->StopTask();
		WatchdogTask = nullptr;
	}

	if (UAbilitySystemComponent* ASC = GetASC())
	{
		ASC->SetLooseGameplayTagCount(KOGameplayTags::State_Character_Guard_PerfectGuard, 0);
		ASC->SetLooseGameplayTagCount(KOGameplayTags::State_Character_Guard_Blocking, 0);
	}

	if (GE_Guard_Reset) ApplyEffectToSelf(GE_Guard_Reset);

	CommitAbilityCooldown(Handle, ActorInfo, ActivationInfo, true);

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

void UKOGA_Guard::OnStaminaChanged()
{
	const float CurrentStamina = GetASC()->GetNumericAttribute(UKOStaminaSet::GetStaminaAttribute());
	if (CurrentStamina <= 0.01f)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
}

void UKOGA_Guard::OnGuardSuccess(FGameplayEventData Data)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;
	
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return;
	
	
	
	const FVector LaunchDir = -Character->GetActorForwardVector();
	FVector FlatLaunchDir = FVector(LaunchDir.X, LaunchDir.Y, 0.f).GetSafeNormal();
	
	FVector LaunchVector = (FlatLaunchDir * LaunchAmount) + FVector(0.f, 0.f, 50.f);
	
	if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
	{
		MoveComp->StopMovementImmediately();
	}
	
	Character->LaunchCharacter(LaunchVector, true,true);
	
	if (ASC->GetCurrentMontage() == GuardMontage)
		ASC->CurrentMontageJumpToSection(FName("GuardUp"));
	
}

void UKOGA_Guard::OnGuardFailed(FGameplayEventData Data)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;
	
	ASC->RemoveLooseGameplayTag(KOGameplayTags::State_Character_Guard_Blocking);
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Guard::OnGuardStart(FGameplayEventData Data)
{
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC) return;
	
	ASC->AddLooseGameplayTag(KOGameplayTags::State_Character_Guard_Blocking);
	if (GE_Guard_Init) ApplyEffectToSelf(GE_Guard_Init); 
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

void UKOGA_Guard::CheckGuardLifetime(float DeltaTime)
{
	if (bWatchdogLogged) return;

	if (GetWorld()->GetTimeSeconds() - ActivationTime > MaxExpectedDuration)
	{
		bWatchdogLogged = true;
		KO_LOG(Combat, Error, TEXT("%s | KOGA_Guard 가 %.1f초 넘게 종료되지 않음 — 강제 종료"),
			*GetAvatarCharacter()->GetName(), MaxExpectedDuration);

		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
