#include "AbilitySystem/Ability/Movement/KOGA_Crouch.h"
#include "GameFramework/Character.h"

UKOGA_Crouch::UKOGA_Crouch()
{
}

bool UKOGA_Crouch::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, 
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false;
		
	return GetAvatarCharacter() && GetAvatarCharacter()->CanCrouch(); 
}

void UKOGA_Crouch::ActivateAbility(
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
	
	ACharacter* Character = GetAvatarCharacter();
	if (!Character) return; 
	
	Character->Crouch();
	
	
}

void UKOGA_Crouch::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	 const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, 
	bool bReplicateEndAbility,
	 bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UKOGA_Crouch::InputReleased(
	const FGameplayAbilitySpecHandle Handle, 
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputReleased(Handle, ActorInfo, ActivationInfo);
}

void UKOGA_Crouch::OnMontageCompleted()
{
}

void UKOGA_Crouch::OnMontageCancelled()
{
}
