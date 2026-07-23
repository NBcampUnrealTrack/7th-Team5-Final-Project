#include "Character/Enemy/Boss/BehaviorTree/BTTask_RotateToTarget.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossController.h"

UBTTask_RotateToTarget::UBTTask_RotateToTarget()
{
	NodeName = TEXT("Rotate To Target");
	bNotifyTick = true;
}
 
EBTNodeResult::Type UBTTask_RotateToTarget::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		return EBTNodeResult::Failed;
	}
 
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}
 
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AKOAIC_BossController::TargetActorKey));
	if (!Target)
	{
		return EBTNodeResult::Failed;
	}
 
	// 이미 플레이어를 바라보고 있으면 즉시 완료
	APawn* Pawn = AIC->GetPawn();
	if (!Pawn)
	{
		return EBTNodeResult::Failed;
	}
 
	FVector Direction = Target->GetActorLocation() - Pawn->GetActorLocation();
	Direction.Z = 0.f;
 
	FRotator TargetRotation = Direction.Rotation();
	FRotator CurrentRotation = Pawn->GetActorRotation();
 
	if (FMath::Abs(FRotator::NormalizeAxis(
		TargetRotation.Yaw - CurrentRotation.Yaw)) <= AngleTolerance)
	{
		return EBTNodeResult::Succeeded;
	}
 
	return EBTNodeResult::InProgress;
}
 
void UBTTask_RotateToTarget::TickTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
 
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
 
	AActor* Target = Cast<AActor>(BB->GetValueAsObject(AKOAIC_BossController::TargetActorKey));
	if (!Target)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
 
	APawn* Pawn = AIC->GetPawn();
	if (!Pawn)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
 
	// 타겟 방향 계산
	FVector Direction = Target->GetActorLocation() - Pawn->GetActorLocation();
	Direction.Z = 0.f;
	if (Direction.IsNearlyZero())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
 
	FRotator TargetRotation = Direction.Rotation();
	FRotator CurrentRotation = Pawn->GetActorRotation();
 
	// 보간 회전
	FRotator NewRotation = FMath::RInterpTo(
		CurrentRotation,
		TargetRotation,
		DeltaSeconds,
		RotateSpeed
	);
 
	Pawn->SetActorRotation(NewRotation);
 
	// 오차 범위 안에 들어오면 완료
	const float AngleDiff = FMath::Abs(FRotator::NormalizeAxis(TargetRotation.Yaw - NewRotation.Yaw));
 
	if (AngleDiff <= AngleTolerance)
	{
		Pawn->SetActorRotation(TargetRotation);
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
