#include "Character/Enemy/Boss/BTService_BossDetectPlayer.h"

#include "AIController.h"
#include "KOAIC_BossChapter01.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"


UBTService_BossDetectPlayer::UBTService_BossDetectPlayer()
{
	Interval = 0.2f;
	NodeName = TEXT("Detect Player");
}

void UBTService_BossDetectPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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
		BB->SetValueAsObject(AKOAIC_BossChapter01::TargetActorKey, PlayerCharacter);
	}
	else
	{
		BB->SetValueAsObject(AKOAIC_BossChapter01::TargetActorKey, nullptr);
	}
	
	// 추적거리 체크
	BB->SetValueAsBool(AKOAIC_BossChapter01::bIsInTraceRangeKey,Distance <= TraceRange);
}
