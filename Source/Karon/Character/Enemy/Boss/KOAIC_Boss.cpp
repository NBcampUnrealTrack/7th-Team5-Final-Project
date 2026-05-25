#include "Character/Enemy/Boss/KOAIC_Boss.h"

#include "BehaviorTree/BehaviorTreeComponent.h"

const FName AKOAIC_Boss::TargetActorKey = TEXT("TargetActor");
const FName AKOAIC_Boss::bIsInTraceRangeKey = TEXT("bIsInTraceRange");
const FName AKOAIC_Boss::bIsInAttackRangeKey = TEXT("bIsInAttackRange");
const FName AKOAIC_Boss::bIsPhase2Key = TEXT("bIsPhase2");
const FName AKOAIC_Boss::bIsGroggyKey = TEXT("bIsGroggy");
 
AKOAIC_Boss::AKOAIC_Boss()
{
}
 
void AKOAIC_Boss::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
 
	if (!BehaviorTree)
	{
		return;
	}
 
	RunBehaviorTree(BehaviorTree);
}
 
void AKOAIC_Boss::OnUnPossess()
{
	Super::OnUnPossess();
 
	if (UBehaviorTreeComponent* BTComp =
		Cast<UBehaviorTreeComponent>(BrainComponent))
	{
		BTComp->StopTree();
	}
}
