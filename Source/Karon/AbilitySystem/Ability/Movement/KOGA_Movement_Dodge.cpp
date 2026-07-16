#include "KOGA_Movement_Dodge.h"

#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UKOGA_Movement_Dodge::UKOGA_Movement_Dodge()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Input_Ability_Movement_Dodge)); 
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Character_Movement_Dodging); 
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_Attacking);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Character_HitReacting);
}

void UKOGA_Movement_Dodge::ActivateAbility(
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
	if (!Character)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	FGameplayTagContainer SprintTag;
	SprintTag.AddTag(KOGameplayTags::Input_Ability_Movement_Sprint);
	GetASC()->CancelAbilities(&SprintTag);
	
	
	UAbilityTask_WaitGameplayEvent* StartEventTask =  
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Dodge_Invincible_Start); 
	
	StartEventTask->EventReceived.AddDynamic(this, &ThisClass::OnStartEventReceived);
	StartEventTask->ReadyForActivation(); 
	
	UAbilityTask_WaitGameplayEvent* EndEventTask =  
		UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, KOGameplayTags::Event_Dodge_Invincible_End); 
	
	EndEventTask->EventReceived.AddDynamic(this, &ThisClass::OnEndEventReceived);
	EndEventTask->ReadyForActivation(); 
	
	FVector Input = CalculateDodgeDirection(); 
	EDodgeDirection Direction = 
		ClassifyDirection(Character->GetActorForwardVector(), Input);
	
	UAnimMontage** Found = DodgeMontages.Find(Direction);
	if (!Found || !*Found)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (!Input.IsNearlyZero())
	{
		Character->LaunchCharacter(Input * LaunchDistance, true, false);
	}
	
	
	UAbilityTask_PlayMontageAndWait* MontageTask 
		= UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, *Found);
	
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageCancelled);
	MontageTask->ReadyForActivation(); 
}

void UKOGA_Movement_Dodge::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (GE_InvincibleHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(GE_InvincibleHandle);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

FVector UKOGA_Movement_Dodge::CalculateDodgeDirection() const
{
	ACharacter* Character = GetAvatarCharacter();
	AController* Controller = GetAvatarController();
	
	if (!Character || !Controller) return FVector::ZeroVector;
	
	const FVector Input = Character->GetCharacterMovement()->GetCurrentAcceleration().GetSafeNormal();

	if (Input.IsNearlyZero())
	{
		return -Character->GetActorForwardVector();
	}
	
	return Input;
}

EDodgeDirection UKOGA_Movement_Dodge::ClassifyDirection(FVector Forward, FVector InputDir) const
{
	float Dot = FVector::DotProduct(Forward, InputDir);    
	float Cross = FVector::CrossProduct(Forward, InputDir).Z; 

	float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(Dot, -1.f, 1.f)));
	if (Cross < 0.f) AngleDeg = -AngleDeg; 
	
	// 양수 = 오른쪽, 음수 = 왼쪽
	if      (AngleDeg >= -22.5f  && AngleDeg <  22.5f)  return EDodgeDirection::Forward;
	else if (AngleDeg >=  22.5f  && AngleDeg <  67.5f)  return EDodgeDirection::ForwardRight;
	else if (AngleDeg >=  67.5f  && AngleDeg < 112.5f)  return EDodgeDirection::Right;
	else if (AngleDeg >= 112.5f  && AngleDeg < 157.5f)  return EDodgeDirection::BackRight;
	else if (AngleDeg >= -67.5f  && AngleDeg < -22.5f)  return EDodgeDirection::ForwardLeft;
	else if (AngleDeg >= -112.5f && AngleDeg < -67.5f)  return EDodgeDirection::Left;
	else if (AngleDeg >= -157.5f && AngleDeg < -112.5f) return EDodgeDirection::BackLeft;
	else                                                return EDodgeDirection::Backward;
}

void UKOGA_Movement_Dodge::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Movement_Dodge::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UKOGA_Movement_Dodge::OnStartEventReceived(FGameplayEventData Data)
{
	GE_InvincibleHandle = ApplyEffectToSelf(GE_Invincible);
}

void UKOGA_Movement_Dodge::OnEndEventReceived(FGameplayEventData Data)
{
	if (GE_InvincibleHandle.IsValid())
	{
		BP_RemoveGameplayEffectFromOwnerWithHandle(GE_InvincibleHandle);
		GE_InvincibleHandle.Invalidate(); 
	}
}
