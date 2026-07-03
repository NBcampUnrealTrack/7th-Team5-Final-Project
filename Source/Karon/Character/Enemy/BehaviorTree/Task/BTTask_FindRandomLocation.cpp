#include "BTTask_FindRandomLocation.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"


UBTTask_FindRandomLocation::UBTTask_FindRandomLocation()
{
	NodeName = TEXT("Find Random Location");

	PatrolLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindRandomLocation, PatrolLocationKey));
	InitialLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_FindRandomLocation, InitialLocationKey));
	MaxDistanceKey.AddFloatFilter(this,GET_MEMBER_NAME_CHECKED(UBTTask_FindRandomLocation, MaxDistanceKey));
}

EBTNodeResult::Type UBTTask_FindRandomLocation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AiController = OwnerComp.GetAIOwner();
	if (AiController == nullptr || AiController->GetPawn() == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	//NavMesh를 가져옵니다.
	const UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSystem == nullptr)
	{
		return EBTNodeResult::Failed;
	}
	
	// 에너미의 초기 위치
	const FVector Origin = OwnerComp.GetBlackboardComponent()->GetValueAsVector(InitialLocationKey.SelectedKeyName);
	float SearchRadius = OwnerComp.GetBlackboardComponent()->GetValueAsFloat(MaxDistanceKey.SelectedKeyName);
	FNavLocation RandomLocation;
	
	//에너미의 초기 위치에서 일정 반경의 무작워 지점
	const bool bFound = NavSystem->GetRandomReachablePointInRadius(Origin, SearchRadius, RandomLocation);
	if (bFound)
	{
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(PatrolLocationKey.SelectedKeyName, RandomLocation.Location);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
