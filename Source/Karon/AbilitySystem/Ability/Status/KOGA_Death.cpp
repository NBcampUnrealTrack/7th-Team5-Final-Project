#include "KOGA_Death.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/KOCharacterBase.h"
#include "GameFramework/Character.h"
#include "Utility/Log/KOLogManager.h"

UKOGA_Death::UKOGA_Death()
{
	InstancingPolicy   = EGameplayAbilityInstancingPolicy::InstancedPerExecution;
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Dead);
	
	FAbilityTriggerData Trigger;
	Trigger.TriggerTag = KOGameplayTags::Event_Death;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
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
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// 어빌리티 캔슬 
	ASC->CancelAllAbilities(this); 
	
	// GE_Death 적용
	if (GE_Death) ApplyEffectToSelf(GE_Death); 
	
	if (TriggerEventData)
		CachedInstigator = const_cast<AActor*>(TriggerEventData->Instigator.Get());
	
	if (!Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}
	
	UAbilityTask_PlayMontageAndWait* Task = 
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, Montage, 1.0f, NAME_None, true);
	
	Task->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	Task->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCompleted);
			
	Task->ReadyForActivation(); 
	
	// GameplayCue 
	ACharacter* Character = GetAvatarCharacter();
	
	FGameplayCueParameters CueParams;
	CueParams.Location =  Character ? Character->GetActorLocation() : FVector::ZeroVector;
	ASC->ExecuteGameplayCue(KOGameplayTags::GameplayCue_Death, CueParams);
}

void UKOGA_Death::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (AKOCharacterBase* Character = Cast<AKOCharacterBase>(GetAvatarCharacter()))
	{
		Character->OnCharacterDead(CachedInstigator.Get());
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Death::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
