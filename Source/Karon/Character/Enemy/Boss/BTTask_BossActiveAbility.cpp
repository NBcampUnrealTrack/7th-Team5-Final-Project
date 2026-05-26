// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Enemy/Boss/BTTask_BossActiveAbility.h"

#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
 
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
 
	// GA 종료 델리게이트 바인딩
	ASC->OnAbilityEnded.AddLambda(
		[this, &OwnerComp](const FAbilityEndedData& Data)
		{
			if (!Data.AbilityThatEnded)
			{
				return;
			}
 
			if (!Data.AbilityThatEnded->GetAssetTags().HasTag(AbilityTag))
			{
				return;
			}
 
			EBTNodeResult::Type Result = Data.bWasCancelled ?
				EBTNodeResult::Failed :
				EBTNodeResult::Succeeded;
 
			FinishLatentTask(OwnerComp, Result);
		}
	);
 
	bool bSuccess = ASC->TryActivateAbilitiesByTag(
		FGameplayTagContainer(AbilityTag)
	);
 
	if (!bSuccess)
	{
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
