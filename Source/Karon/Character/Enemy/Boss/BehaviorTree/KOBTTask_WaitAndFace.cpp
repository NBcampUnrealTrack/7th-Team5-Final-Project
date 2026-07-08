#include "Character/Enemy/Boss/BehaviorTree/KOBTTask_WaitAndFace.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossController.h"

UKOBTTask_WaitAndFace::UKOBTTask_WaitAndFace()
{
	NodeName = TEXT("Wait And Face Target");
	bNotifyTick = true;
}
 
EBTNodeResult::Type UKOBTTask_WaitAndFace::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	bWaitFinished = false;

	UBehaviorTreeComponent* OwnerCompPtr = &OwnerComp;
	GetWorld()->GetTimerManager().SetTimer(
		WaitTimerHandle,
		FTimerDelegate::CreateLambda([this, OwnerCompPtr]()
		{
			if (!OwnerCompPtr) 
			{ 
				return; 
			}
			bWaitFinished = true;
			FinishLatentTask(*OwnerCompPtr, EBTNodeResult::Succeeded);
		}),
		WaitDuration,
		false
	);
 
	return EBTNodeResult::InProgress;
}
 
void UKOBTTask_WaitAndFace::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	if (bWaitFinished)
	{
		return;
	}
 
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		return;
	}
 
	APawn* Pawn = AIC->GetPawn();
	if (!Pawn)
	{
		return;
	}
 
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return;
	}
 
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(FaceTargetKey.SelectedKeyName));
	if (!Target)
	{
		return;
	}
	
	FVector Direction = Target->GetActorLocation() - Pawn->GetActorLocation();
	Direction.Z = 0.f;
	if (Direction.IsNearlyZero())
	{
		return;
	}
 
	FRotator NewRotation = FMath::RInterpTo(
		Pawn->GetActorRotation(),
		Direction.Rotation(),
		DeltaSeconds,
		RotateSpeed
	);
 
	Pawn->SetActorRotation(NewRotation);
}
 
void UKOBTTask_WaitAndFace::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	GetWorld()->GetTimerManager().ClearTimer(WaitTimerHandle);
	bWaitFinished = false;
 
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
