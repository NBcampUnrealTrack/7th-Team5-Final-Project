#include "Character/Enemy/Boss/KOBTService_BossDetectPlayer.h"

#include "AIController.h"
#include "KOAIC_Boss.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


UKOBTService_BossDetectPlayer::UKOBTService_BossDetectPlayer()
{
	Interval = 0.2f;
	NodeName = TEXT("Detect Player");
}

void UKOBTService_BossDetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
	
	AAIController* AIC = OwnerComp.GetAIOwner();
	if (!AIC)
	{
		return;
	}
	
	APawn* BossPawn = AIC->GetPawn();
	if (!BossPawn)
	{
		return;
	}
	
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
	if (!BB)
	{
		return;
	}
	
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(BossPawn, 0);
	if (!PlayerCharacter)
	{
		return;
	}
	
	const float Distance = FVector::Dist(BossPawn->GetActorLocation(), PlayerCharacter->GetActorLocation());
	
	if (Distance <= TraceRange)
	{
		BB->SetValueAsObject(AKOAIC_Boss::TargetActorKey, PlayerCharacter);
	}
	else
	{
		BB->SetValueAsObject(AKOAIC_Boss::TargetActorKey, nullptr);
	}
	
	// 추적거리 체크
	BB->SetValueAsBool(AKOAIC_Boss::bIsInTraceRangeKey,Distance <= TraceRange);
 
	// 공격거리 체크
	BB->SetValueAsBool(AKOAIC_Boss::bIsInAttackRangeKey,Distance <= AttackRange);
}
