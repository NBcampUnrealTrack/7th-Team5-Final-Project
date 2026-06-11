#include "KOGA_Movement_Jump.h"
#include "Abilities/Tasks/AbilityTask_WaitMovementModeChange.h"
#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UKOGA_Movement_Jump::UKOGA_Movement_Jump()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	 SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Movement_Jump)); 
}

bool UKOGA_Movement_Jump::CanActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, 
	FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)) return false; 
	
	ACharacter* Character =GetAvatarCharacter();
	return Character && Character->ACharacter::CanJump(); 
}

void UKOGA_Movement_Jump::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AKOHeroCharacter* Character = Cast<AKOHeroCharacter>(GetAvatarCharacter()); 
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}
	
	UCharacterMovementComponent* CMC = Character->GetCharacterMovement();
	if (!CMC)
	{
		Character->Jump(); 
	}
	
	// TODO: 
	// 1. 점프 분기 (타겟팅 하면서 점프) 
	// 2. 파쿠르 
	// 3. 그냥 점프 
	
	UKOMovementSet* MovementSet = Character->GetMovementSet();
	float JumpStrength = MovementSet ? MovementSet->GetJumpStrength() : 600; 
	
	FVector DirectionalJump =CMC->Velocity* 0.6f + FVector(0.f, 0.f, JumpStrength);
	
	Character->LaunchCharacter(DirectionalJump,true, true); 
	
	UAbilityTask_WaitMovementModeChange* WaitLandTask =
		UAbilityTask_WaitMovementModeChange::CreateWaitMovementModeChange(this, MOVE_Walking); 
	WaitLandTask->OnChange.AddDynamic(this, &ThisClass::OnCharacterLanded);
	WaitLandTask->ReadyForActivation();
}

void UKOGA_Movement_Jump::OnCharacterLanded(EMovementMode NewMovementMode)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
