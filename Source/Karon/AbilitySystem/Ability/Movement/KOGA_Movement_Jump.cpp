#include "KOGA_Movement_Jump.h"
#include "Abilities/Tasks/AbilityTask_WaitMovementModeChange.h"
#include "GameFramework/Character.h"

UKOGA_Movement_Jump::UKOGA_Movement_Jump()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	// SetAssetTags()
}

bool UKOGA_Movement_Jump::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, 
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false; 
	
	ACharacter* Character = GetAvatarCharacter();
	
	return Character && Character->ACharacter::CanJump(); 
}

void UKOGA_Movement_Jump::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	ACharacter* Character = GetAvatarCharacter();
	Character->ACharacter::Jump();
	
	UAbilityTask_WaitMovementModeChange* WaitLandTask =
		UAbilityTask_WaitMovementModeChange::CreateWaitMovementModeChange(this, MOVE_Walking); 
	WaitLandTask->OnChange.AddDynamic(this, &ThisClass::OnCharacterLanded);
	WaitLandTask->ReadyForActivation();
}

void UKOGA_Movement_Jump::OnCharacterLanded(EMovementMode NewMovementMode)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
