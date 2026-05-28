#include "KOGA_Death.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "GameFramework/Character.h"


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
	
	UE_LOG(LogTemp,Warning,TEXT("[GA_Death] : Activated.")); 
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		UE_LOG(LogTemp,Warning,TEXT("[GA_Death] : Fail to Commit."));
		return;
	}
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (!ASC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		UE_LOG(LogTemp,Warning,TEXT("[GA_Death] : ASC is Null."));
		return;
	}
	
	// 어빌리티 캔슬 
	ASC->CancelAllAbilities(); 
	
	// GE_Death 적용
	if (GE_Death)
	{
		ApplyEffectToSelf(GE_Death); 
		UE_LOG(LogTemp,Warning,TEXT("[GA_Death] : ApplyEffectToSelf."));
	}
	else
	{
		UE_LOG(LogTemp,Warning,TEXT("[GA_Death] : Fail to ApplyEffectToSelf."));
	}
	

	// 몽타주 재생 
	if (Montage)
	{
		UAbilityTask_PlayMontageAndWait* Task = 
			UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
			NAME_None,
			Montage,
			1.0f,
			NAME_None,
			true
			);
		
		if (Task)
		{
			Task->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
			Task->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCompleted);
			
			Task->ReadyForActivation(); 
		}
	}
	
	// GameplayCue 
	ACharacter* Character = GetAvatarCharacter();
	
	FGameplayCueParameters CueParams;
	CueParams.Location =  Character ? Character->GetActorLocation() : FVector::ZeroVector;
	ASC->ExecuteGameplayCue(KOGameplayTags::GameplayCue_Death, CueParams);
}

void UKOGA_Death::EndAbility(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, 
	bool bReplicateEndAbility, 
	bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Death::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
