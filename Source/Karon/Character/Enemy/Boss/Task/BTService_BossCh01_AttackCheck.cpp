#include "BTService_BossCh01_AttackCheck.h"

#include "AIController.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossChapter01.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

 
UBTService_BossCh01_AttackCheck::UBTService_BossCh01_AttackCheck()
{
	Interval = 0.1f;
	NodeName = TEXT("Boss Attack Checker");
}
 
void UBTService_BossCh01_AttackCheck::TickNode(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	float DeltaSeconds)
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
 
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(BossPawn);
	if (!ASCInterface)
	{
		return;
	}
 
	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}
 
	ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(BossPawn, 0);
	if (!PlayerCharacter)
	{
		return;
	}
	
	// 거리 계산
	const float Distance = FVector::Dist(
		BossPawn->GetActorLocation(),
		PlayerCharacter->GetActorLocation()
	);
	
	const bool bIsAttacking = ASC->HasMatchingGameplayTag(KOGameplayTags::State_Boss_Attacking);

	for (const FBossAttackInfo& Info : AttackInfos)
	{
		const bool bOnCooldown = ASC->HasMatchingGameplayTag(Info.CooldownTag);
 
		BB->SetValueAsBool(
			Info.BBKey,
			!bIsAttacking && !bOnCooldown && Distance <= Info.AttackRange
		);
	}

	BB->SetValueAsBool(AKOAIC_BossChapter01::bIsAttackingKey, bIsAttacking);
}
