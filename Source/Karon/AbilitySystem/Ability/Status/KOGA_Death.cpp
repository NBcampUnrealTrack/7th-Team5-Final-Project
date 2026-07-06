#include "KOGA_Death.h"

#include "AbilitySystemComponent.h"
#include "KOGA_HitReact.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/KOCharacterBase.h"
#include "GameFramework/Character.h"
#include "Utility/Log/KOLogManager.h"

UKOGA_Death::UKOGA_Death()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_Dead); 
	
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = KOGameplayTags::Event_Death;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

bool UKOGA_Death::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, 
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false; 
	}
	
	AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter()); 
	if (!Character) return false; 

	return Character->bIsDead == false; // 죽지 않은 경우에만 실행 
}

void UKOGA_Death::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, 
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return;
	}
	
	UAbilitySystemComponent* ASC = GetASC();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	Character->OnCharacterDead(CachedInstigator.Get());
	
	
	// 어빌리티 캔슬 
	ASC->CancelAllAbilities(this);
	
	// GE_Death 적용
	if (GE_Death) ApplyEffectToSelf(GE_Death); 
	
	if (TriggerEventData)
		CachedInstigator = const_cast<AActor*>(TriggerEventData->Instigator.Get());
	
	if (!DeathMontage)
	{
		KO_LOGS(GAS, Ability, Warning, TEXT("Death Ability has no Montage."));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		return; 
	}
	
	UAbilityTask_WaitGameplayEvent* WaitEvent = 
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
			this, KOGameplayTags::Event_Montage_Finish, nullptr, true);
	
	WaitEvent->EventReceived.AddDynamic(this, &ThisClass::OnReceiveEvent);
	WaitEvent->ReadyForActivation(); 
	
	UAbilityTask_PlayMontageAndWait* Task = 
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, DeathMontage, 1.0f, NAME_None, false);
	
	Task->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
	Task->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	Task->ReadyForActivation(); 
	
	KO_LOGS(GAS, Ability, Warning, TEXT("Death Ability play Montage."));
	
	// Gameplay Cue 
	FGameplayCueParameters CueParams;
	CueParams.Location = Character->GetActorLocation();
	ASC->ExecuteGameplayCue(KOGameplayTags::GameplayCue_Death, CueParams);
}

void UKOGA_Death::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Death::OnReceiveEvent(FGameplayEventData Payload)
{
	KO_LOGS(GAS, Ability, Warning, TEXT("Received Tag in Death Montage"));
	
	if (ACharacter* Character = GetAvatarCharacter())
	{
		Character->GetMesh()->bPauseAnims = true; 
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,true, false);
}

void UKOGA_Death::OnMontageCompleted()
{
	KO_LOGS(GAS, Ability, Warning, TEXT("Death Montage Completed"));

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,true, false);
}

void UKOGA_Death::OnMontageInterrupted()
{
	KO_LOGS(GAS, Ability, Warning, TEXT("Death Montage Interrupted"));
	
	if (ACharacter* Character = GetAvatarCharacter())
	{
		UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance();
		UAnimMontage* Current = AnimInst ? AnimInst->GetCurrentActiveMontage() : nullptr;
		KO_LOG(GAS, Warning, TEXT("Interrupted by Montage: %s"),
			Current ? *Current->GetName() : TEXT("None"));
	}
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo,true, false);
}

