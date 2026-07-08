#include "Character/Enemy/Boss/KOAIC_BossController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"

const FName AKOAIC_BossController::TargetActorKey = TEXT("TargetActor");
const FName AKOAIC_BossController::bIsInTraceRangeKey = TEXT("bIsInTraceRange");
const FName AKOAIC_BossController::bIsPhase2Key = TEXT("bIsPhase2");
const FName AKOAIC_BossController::bIsGroggyKey = TEXT("bIsGroggy");
const FName AKOAIC_BossController::bIsAttackingKey = TEXT("bIsAttacking");
const FName AKOAIC_BossController::bIsDeadKey = TEXT("bIsDead");
const FName AKOAIC_BossController::GroundHitDirCheckKey = TEXT("GroundHitDirCheck");
const FName AKOAIC_BossController::bIsGimmickReadyKey= TEXT("bIsGimmickReady");
const FName AKOAIC_BossController::GimmickLocationKey = TEXT("GimmickLocation");

 
AKOAIC_BossController::AKOAIC_BossController()
{
}
 
void AKOAIC_BossController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
 
	if (!BehaviorTree)
	{
		return;
	}
 
	RunBehaviorTree(BehaviorTree);
}
 
void AKOAIC_BossController::OnUnPossess()
{
	Super::OnUnPossess();
 
	if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent))
	{
		BTComp->StopTree();
	}
}
