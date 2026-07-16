#include "KOGA_Movement_Jump_LockOn.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitMovementModeChange.h"
#include "AbilitySystem/Attribute/KOMovementSet.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"
#include "Character/Hero/KOHeroCharacter.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UKOGA_Movement_Jump_LockOn::UKOGA_Movement_Jump_LockOn()
{
	ActivationRequiredTags.AddTag(KOGameplayTags::State_Character_LockOn); 
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Attacking);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Movement_Dodging);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_HitReacting);
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = KOGameplayTags::Event_Movement_Jump_LockOn;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UKOGA_Movement_Jump_LockOn::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, 
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	AKOHeroCharacter* Character = Cast<AKOHeroCharacter>(GetAvatarCharacter());
	if (!Character)
	{
		EndAbility(Handle, ActorInfo,ActivationInfo, true, true);
		return;
	}
	
	UCharacterMovementComponent* CMC = Character->GetCharacterMovement();
	if (!CMC)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	EJumpDirection Direction = DetermineJumpDirection(Character);
	
	UAnimMontage* SelectedMontage = JumpMontages.FindRef(Direction);
	if (!SelectedMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return; 
	}
	
	UAbilityTask_PlayMontageAndWait* MontageTask =
		UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		  this, NAME_None, SelectedMontage, 1.0f, NAME_None, false);
	
	MontageTask->ReadyForActivation();
	
	UKOMovementSet* MovementSet = Character->GetMovementSet();
	float JumpStrength = MovementSet ? MovementSet->GetJumpStrength() : 600.f;
	FVector DirectionalJump = CMC->Velocity * 0.6f + FVector(0.f, 0.f, JumpStrength);
	Character->LaunchCharacter(DirectionalJump, true, true);
	
	UAbilityTask_WaitMovementModeChange* WaitLandTask =
	   UAbilityTask_WaitMovementModeChange::CreateWaitMovementModeChange(this, MOVE_Walking);
	WaitLandTask->OnChange.AddDynamic(this, &ThisClass::OnCharacterLanded);
	WaitLandTask->ReadyForActivation();
}

void UKOGA_Movement_Jump_LockOn::OnCharacterLanded(EMovementMode NewMovementMode)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

EJumpDirection UKOGA_Movement_Jump_LockOn::DetermineJumpDirection(ACharacter* Character) const
{
	FVector HorizontalVelocity = Character->GetVelocity();
	HorizontalVelocity.Z = 0.f;

	if (HorizontalVelocity.SizeSquared() < IdleSpeedThreshold * IdleSpeedThreshold)
		return EJumpDirection::Idle;

	const FVector VelocityDir = HorizontalVelocity.GetSafeNormal();
	const float DotForward = FVector::DotProduct(Character->GetActorForwardVector(), VelocityDir);

	if (DotForward > ForwardThreshold)  return EJumpDirection::Forward;
	if (DotForward < -ForwardThreshold) return EJumpDirection::Backward;

	const float DotRight = FVector::DotProduct(Character->GetActorRightVector(), VelocityDir);
	return DotRight > 0.f ? EJumpDirection::Right : EJumpDirection::Left;
}

