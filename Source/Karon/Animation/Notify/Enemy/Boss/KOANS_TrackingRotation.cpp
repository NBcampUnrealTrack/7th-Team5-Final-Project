#include "Animation/Notify/Enemy/Boss/KOANS_TrackingRotation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossController.h"

void UKOANS_TrackingRotation::NotifyTick(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner()) 
	{
		return; 
	}
 
	APawn* Pawn = Cast<APawn>(MeshComp->GetOwner());
	if (!Pawn) 
	{ 
		return; 
	}
 
	AAIController* AIC = Cast<AAIController>(Pawn->GetController());
	if (!AIC) 
	{ 
		return; 
	}
 
	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!BB) 
	{ 
		return; 
	}
 
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AKOAIC_BossController::TargetActorKey));
	if (!Target) 
	{ 
		return; 
	}
 
	// Z 무시하고 수평 방향만 회전
	FVector Direction = Target->GetActorLocation()
		- Pawn->GetActorLocation();
	Direction.Z = 0.f;
	if (Direction.IsNearlyZero()) 
	{ 
		return; 
	}
 
	FRotator NewRotation = FMath::RInterpTo(
		Pawn->GetActorRotation(),
		Direction.Rotation(),
		FrameDeltaTime,
		RotateSpeed
	);
 
	Pawn->SetActorRotation(NewRotation);
}
