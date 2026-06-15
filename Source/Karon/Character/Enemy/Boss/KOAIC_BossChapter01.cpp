#include "Character/Enemy/Boss/KOAIC_BossChapter01.h"

#include "BehaviorTree/BehaviorTreeComponent.h"

const FName AKOAIC_BossChapter01::TargetActorKey = TEXT("TargetActor");
const FName AKOAIC_BossChapter01::bIsInTraceRangeKey = TEXT("bIsInTraceRange");
const FName AKOAIC_BossChapter01::bIsPhase2Key = TEXT("bIsPhase2");
const FName AKOAIC_BossChapter01::bIsGroggyKey = TEXT("bIsGroggy");
const FName AKOAIC_BossChapter01::bIsAttackingKey = TEXT("bIsAttacking");
const FName AKOAIC_BossChapter01::bIsDeadKey = TEXT("bIsDead");
const FName AKOAIC_BossChapter01::GroundHitDirCheckKey = TEXT("GroundHitDirCheck");
const FName AKOAIC_BossChapter01::bIsGimmickReadyKey= TEXT("bIsGimmickReady");
const FName AKOAIC_BossChapter01::GimmickLocationKey = TEXT("GimmickLocation");

 
AKOAIC_BossChapter01::AKOAIC_BossChapter01()
{
}
 
void AKOAIC_BossChapter01::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
 
	if (!BehaviorTree)
	{
		return;
	}
 
	RunBehaviorTree(BehaviorTree);
}
 
void AKOAIC_BossChapter01::OnUnPossess()
{
	Super::OnUnPossess();
 
	if (UBehaviorTreeComponent* BTComp = Cast<UBehaviorTreeComponent>(BrainComponent))
	{
		BTComp->StopTree();
	}
}
