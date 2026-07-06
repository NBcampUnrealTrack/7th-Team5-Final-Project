#include "KOGA_StaminaExhausted.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayTag.h"
#include "AbilitySystem/Attribute/KOStaminaSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Animation/KOAnimationTypes.h"
#include "Character/Hero/KOHeroCharacter.h"

UKOGA_StaminaExhausted::UKOGA_StaminaExhausted()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_StaminaExhausted);
	
	// 트리거 설정 
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = KOGameplayTags::Event_Stamina_Exhausted;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
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
		ExhaustedEffectHandle = ApplyEffectSetByCallerToSelf(
			ExhaustedEffect,
			KOGameplayTags::Data_Attribute_Movement_WalkSpeed,
			ExhaustedSpeed
		);
	}
	
	AKOHeroCharacter* Character = Cast<AKOHeroCharacter>(GetAvatarCharacter());
	if (Character) Character->UpdateGait(EGait::Walk);
	
	UAbilityTask_WaitGameplayEvent* WaitStaminaFull =
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Stamina_Full);
	
	if (WaitStaminaFull)
	{
		WaitStaminaFull->EventReceived.AddDynamic(this, &ThisClass::OnStaminaFull);
		WaitStaminaFull->ReadyForActivation();
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
	
	if (AKOHeroCharacter* Character = Cast<AKOHeroCharacter>(GetAvatarCharacter()))
			Character->UpdateGait(EGait::Run);
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_StaminaExhausted::OnStaminaFull(FGameplayEventData Data)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
