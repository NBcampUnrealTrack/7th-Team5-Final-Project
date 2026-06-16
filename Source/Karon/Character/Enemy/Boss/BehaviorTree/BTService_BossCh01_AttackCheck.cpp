#include "BTService_BossCh01_AttackCheck.h"

#include "AIController.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossChapter01.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "Character/Enemy/Boss/KOBossDataAsset.h"
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
	
	AKOBossBase* Boss = Cast<AKOBossBase>(BossPawn);
	UKOBossDataAsset* DA = Boss ? Boss->GetDataAsset() : nullptr;
	
	const float Distance = FVector::Dist(BossPawn->GetActorLocation(), PlayerCharacter->GetActorLocation());
	
	const bool bIsAttacking = ASC->HasMatchingGameplayTag(
		KOGameplayTags::State_Boss_Attacking);
 
	for (const FBossAttackInfo& Info : AttackInfos)
	{
		const bool bOnCooldown = ASC->HasMatchingGameplayTag(Info.CooldownTag);
		
		const float AttackRange = DA ? DA->GetAttackRange(Info.AttackAbilityClass) : 0.f;
		
		const bool bResult = !bIsAttacking && !bOnCooldown && Distance <= AttackRange;
		
		BB->SetValueAsBool(Info.BBKey, bResult);
	}

	BB->SetValueAsBool(AKOAIC_BossChapter01::bIsAttackingKey, bIsAttacking);
	
	// 플레이어 방향 판별
	const FVector ToPlayer = (PlayerCharacter->GetActorLocation() - BossPawn->GetActorLocation()).GetSafeNormal();
 
	const float DotRight = FVector::DotProduct(BossPawn->GetActorRightVector(), ToPlayer);
	
	BB->SetValueAsBool(AKOAIC_BossChapter01::GroundHitDirCheckKey, DotRight < 0.f);
}
