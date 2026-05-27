#include "KOGA_StaminaExhausted.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "AbilitySystem/Attribute/KOStaminaSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

UKOGA_StaminaExhausted::UKOGA_StaminaExhausted()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_StaminaExhausted);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_StaminaExhausted);
	
	// 트리거 설정 
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = KOGameplayTags::Event_Stamina_Exhausted;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::OwnedTagAdded;
	AbilityTriggers.Add(TriggerData);
}

void UKOGA_StaminaExhausted::ActivateAbility(
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
	
	// 탈진 GE 적용 
	if (ExhaustedEffect)
	{
		FGameplayEffectSpecHandle Spec = MakeOutgoingGameplayEffectSpec(ExhaustedEffect);
		ExhaustedEffectHandle = ApplyGameplayEffectSpecToOwner(
			Handle, ActorInfo, ActivationInfo, Spec);
	}
	
	UAbilityTask_WaitGameplayTagRemoved* WaitTask =
		UAbilityTask_WaitGameplayTagRemoved::WaitGameplayTagRemove(
		this,
		KOGameplayTags::Event_Stamina_Exhausted
	);
	
	if (WaitTask)
	{
		WaitTask->Removed.AddDynamic(this, &ThisClass::OnExhaustedTagRemoved);
		WaitTask->ReadyForActivation();
	}
}

void UKOGA_StaminaExhausted::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, 
	bool bWasCancelled
	)
{
	// Exhaust 디버프 제거 
	if (ExhaustedEffectHandle.IsValid())
		BP_RemoveGameplayEffectFromOwnerWithHandle(ExhaustedEffectHandle);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_StaminaExhausted::OnExhaustedTagRemoved()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
