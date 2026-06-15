#include "Character/Enemy/Boss/BehaviorTree/KOBTTask_MoveToTaggedLocation.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"

UKOBTTask_MoveToTaggedLocation::UKOBTTask_MoveToTaggedLocation()
{
	NodeName = TEXT("Move To Tagged Location");
	bNotifyTaskFinished = true;
 
	TargetLocationKey.AddVectorFilter(
		this,
		GET_MEMBER_NAME_CHECKED(UKOBTTask_MoveToTaggedLocation, TargetLocationKey)
	);
}
 
EBTNodeResult::Type UKOBTTask_MoveToTaggedLocation::ExecuteTask(
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
 
	if (TargetLocationKey.SelectedKeyName.IsNone())
	{
		return EBTNodeResult::Failed;
	}
 
	const FVector TargetLocation = BB->GetValueAsVector(TargetLocationKey.SelectedKeyName);
	
	CachedOwnerComp = &OwnerComp;
	AIC->ReceiveMoveCompleted.AddDynamic(this, &UKOBTTask_MoveToTaggedLocation::OnMoveCompleted);
 
	FAIMoveRequest MoveReq(TargetLocation);
	MoveReq.SetAcceptanceRadius(AcceptanceRadius);
 
	FPathFollowingRequestResult Result = AIC->MoveTo(MoveReq);
 
	if (Result.Code == EPathFollowingRequestResult::Failed)
	{
		AIC->ReceiveMoveCompleted.RemoveDynamic(this, &UKOBTTask_MoveToTaggedLocation::OnMoveCompleted);
		return EBTNodeResult::Failed;
	}
	
	if (Result.Code == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		AIC->ReceiveMoveCompleted.RemoveDynamic(this, &UKOBTTask_MoveToTaggedLocation::OnMoveCompleted);
		return EBTNodeResult::Succeeded;
	}
 
	return EBTNodeResult::InProgress;
}
 
void UKOBTTask_MoveToTaggedLocation::OnMoveCompleted(
	FAIRequestID RequestID,
	EPathFollowingResult::Type Result)
{
	if (!CachedOwnerComp)
	{
		return;
	}
 
	AAIController* AIC = CachedOwnerComp->GetAIOwner();
	if (AIC)
	{
		AIC->ReceiveMoveCompleted.RemoveDynamic(this, &UKOBTTask_MoveToTaggedLocation::OnMoveCompleted);
	}
 
	const EBTNodeResult::Type BTResult =
		(Result == EPathFollowingResult::Success) ?
		EBTNodeResult::Succeeded : EBTNodeResult::Failed;
 
	FinishLatentTask(*CachedOwnerComp, BTResult);
}
 
void UKOBTTask_MoveToTaggedLocation::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (AIC)
	{
		AIC->ReceiveMoveCompleted.RemoveDynamic(this, &UKOBTTask_MoveToTaggedLocation::OnMoveCompleted);
	}
 
	CachedOwnerComp = nullptr;
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
 
