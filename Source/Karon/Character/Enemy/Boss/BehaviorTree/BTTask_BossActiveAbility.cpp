#include "BTTask_BossActiveAbility.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Enemy/Boss/KOAIC_BossController.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

UBTTask_BossActiveAbility::UBTTask_BossActiveAbility()
{
	NodeName = TEXT("Activate Boss Ability");
	bNotifyTaskFinished = true;
}
 
EBTNodeResult::Type UBTTask_BossActiveAbility::ExecuteTask(
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
 
	IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(BossPawn);
	if (!ASCInterface)
	{
		return EBTNodeResult::Failed;
	}
 
	UAbilitySystemComponent* ASC = ASCInterface->GetAbilitySystemComponent();
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}
 
	if (!AbilityTag.IsValid())
	{
		return EBTNodeResult::Failed;
	}

	if (AKOBossBase* Boss = Cast<AKOBossBase>(BossPawn))
	{
		if (UBlackboardComponent* BB = AIC->GetBlackboardComponent())
		{
			Boss->CurrentTarget = Cast<AActor>(
				BB->GetValueAsObject(AKOAIC_BossController::TargetActorKey));

			if (AbilityTag == KOGameplayTags::State_BossCH01_Attack_Jump)
			{
				if (AActor* Target = Boss->CurrentTarget)
				{
					Boss->JumpTargetLocation = Target->GetActorLocation();
				}
			}
		}
	}
	
	CachedOwnerComp = &OwnerComp;
	ASC->OnAbilityEnded.AddUObject(this, &UBTTask_BossActiveAbility::OnAbilityEndedCallback);
	
	bool bSuccess = ASC->TryActivateAbilitiesByTag(AbilityTag.GetSingleTagContainer());
 
	if (!bSuccess)
	{
		ASC->OnAbilityEnded.RemoveAll(this);
		CachedOwnerComp = nullptr;
		return EBTNodeResult::Failed;
	}
 
	return EBTNodeResult::InProgress;
}
 
void UBTTask_BossActiveAbility::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_BossActiveAbility::OnAbilityEndedCallback(const FAbilityEndedData& Data)
{
	if (!Data.AbilityThatEnded)
	{
		return;
	}
	
	if (!Data.AbilityThatEnded->GetAssetTags().HasTag(AbilityTag))
	{
		return;
	}
	
	if (!CachedOwnerComp)
	{
		return;
	}

	const EBTNodeResult::Type Result = Data.bWasCancelled ?
		EBTNodeResult::Failed : EBTNodeResult::Succeeded;

	FinishLatentTask(*CachedOwnerComp, Result);
}
