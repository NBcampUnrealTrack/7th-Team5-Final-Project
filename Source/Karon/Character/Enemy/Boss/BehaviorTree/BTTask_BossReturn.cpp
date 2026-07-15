#include "Character/Enemy/Boss/BehaviorTree/BTTask_BossReturn.h"

#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "Character/Enemy/Boss/KOAIC_BossController.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_BossReturn::UBTTask_BossReturn()
{
	NodeName = TEXT("Boss Return To Spawn");
	bNotifyTaskFinished = true;
}
 
EBTNodeResult::Type UBTTask_BossReturn::ExecuteTask(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory)
{
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		return EBTNodeResult::Failed;
	}
	
	APawn* BossPawn = AIC->GetPawn();
	if (!BossPawn)
	{
		return EBTNodeResult::Failed;
	}
	
	UBlackboardComponent* BB = AIC->GetBlackboardComponent();
	if (!BB)
	{
		return EBTNodeResult::Failed;
	}
	
	FVector SpawnLocation = BB->GetValueAsVector(AKOAIC_BossController::SpawnLocationKey);

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (NavSys)
	{
		FNavLocation ProjectedLocation;
		if (NavSys->ProjectPointToNavigation(
			SpawnLocation, ProjectedLocation, FVector(500.f, 500.f, 500.f)))
		{
			SpawnLocation = ProjectedLocation.Location;
		}
	}
	
	if (AKOBossBase* Boss = Cast<AKOBossBase>(BossPawn))
	{
		Boss->NotifyPlayerLost();
	}
 
	CachedOwnerComp = &OwnerComp;
	
	AIC->GetPathFollowingComponent()->OnRequestFinished.AddUObject(this, &UBTTask_BossReturn::OnMoveCompleted);
	
	const EPathFollowingRequestResult::Type MoveResult = AIC->MoveToLocation(SpawnLocation, AcceptanceRadius, true);
	
	if (MoveResult == EPathFollowingRequestResult::Failed)
	{
		AIC->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
		CachedOwnerComp = nullptr;
		return EBTNodeResult::Failed;
	}
 
	if (MoveResult == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		if (AKOBossBase* Boss = Cast<AKOBossBase>(BossPawn))
		{
			Boss->RestoreToFull();
		}
		BB->SetValueAsBool(AKOAIC_BossController::bIsReturnKey, false);
		AIC->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
		CachedOwnerComp = nullptr;
		return EBTNodeResult::Succeeded;
	}
 
	return EBTNodeResult::InProgress;
}
 
void UBTTask_BossReturn::OnMoveCompleted(
	FAIRequestID RequestID,
	const FPathFollowingResult& Result)
{
	if (!CachedOwnerComp)
	{
		return;
	}
	
	AAIController* AIC = CachedOwnerComp->GetAIOwner();
	if (AIC)
	{
		AIC->GetPathFollowingComponent()->OnRequestFinished.RemoveAll(this);
		
		if (Result.IsSuccess())
		{
			if (AKOBossBase* Boss = Cast<AKOBossBase>(AIC->GetPawn()))
			{
				Boss->RestoreToFull();
			}
 
			if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
			{
				BB->SetValueAsBool(AKOAIC_BossController::bIsReturnKey, false);
			}
		}
	}
 
	const EBTNodeResult::Type BTResult =
		Result.IsSuccess() ?
		EBTNodeResult::Succeeded : EBTNodeResult::Failed;
 
	FinishLatentTask(*CachedOwnerComp, BTResult);
	CachedOwnerComp = nullptr;
}
 
void UBTTask_BossReturn::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	if (AAIController* AIC = OwnerComp.GetAIOwner())
	{
		if (UPathFollowingComponent* PFC = AIC->GetPathFollowingComponent())
		{
			PFC->OnRequestFinished.RemoveAll(this);
		}
		AIC->StopMovement();
	}
 
	CachedOwnerComp = nullptr;
 
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
