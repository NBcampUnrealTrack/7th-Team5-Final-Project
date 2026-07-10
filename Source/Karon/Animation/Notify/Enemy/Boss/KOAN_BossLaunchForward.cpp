#include "Animation/Notify/Enemy/Boss/KOAN_BossLaunchForward.h"

#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

void UKOAN_BossLaunchForward::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}
	
	ACharacter* Character = Cast<ACharacter>(MeshComp->GetOwner());
	if (!Character)
	{
		return;
	}
	
	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	const float OriginalFriction = MoveComp->GroundFriction;
	const float OriginalBraking = MoveComp->BrakingDecelerationWalking;
 
	MoveComp->GroundFriction = 0.f;
	MoveComp->BrakingDecelerationWalking = 0.f;
 
	UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance();
	const ERootMotionMode::Type OriginalRootMotionMode =
		AnimInst ? AnimInst->RootMotionMode.GetValue() : ERootMotionMode::RootMotionFromEverything;
 
	if (AnimInst)
	{
		AnimInst->RootMotionMode = ERootMotionMode::IgnoreRootMotion;
	}
 
	const FVector ForwardDir = Character->GetActorForwardVector();
	const FVector LaunchVelocity =
		ForwardDir * LaunchSpeed +
		FVector::UpVector * LaunchUpSpeed;
 
	Character->LaunchCharacter(LaunchVelocity, bOverrideXY, bOverrideZ);
 
	FTimerHandle RestoreTimerHandle;
	Character->GetWorldTimerManager().SetTimer(
		RestoreTimerHandle,
		FTimerDelegate::CreateLambda([MoveComp, AnimInst, OriginalFriction, OriginalBraking, OriginalRootMotionMode]()
		{
			if (MoveComp)
			{
				MoveComp->GroundFriction = OriginalFriction;
				MoveComp->BrakingDecelerationWalking = OriginalBraking;
			}
			if (AnimInst)
			{
				AnimInst->RootMotionMode = OriginalRootMotionMode;
			}
		}),
		FrictionRestoreDelay,
		false
	);
}
