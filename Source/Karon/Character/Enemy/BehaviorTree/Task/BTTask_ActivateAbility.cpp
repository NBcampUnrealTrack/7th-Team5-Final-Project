// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ActivateAbility.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "GameFramework/Character.h"


UBTTask_ActivateAbility::UBTTask_ActivateAbility()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_ActivateAbility::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	
	AAIController* Owner = OwnerComp.GetAIOwner();
	if (Owner == nullptr)
	{
		return EBTNodeResult::Aborted;
	}

	ACharacter* AICharacter = Cast<ACharacter>(Owner->GetPawn());
	if (AICharacter == nullptr || ActivateTagName == FGameplayTag::EmptyTag)
	{
		return EBTNodeResult::Failed;
	}
	AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(AICharacter);
	if(Enemy==nullptr)
	{
		return EBTNodeResult::Failed;
	}
	
	if (Enemy->GetAbilitySystemComponent())
	{
		FGameplayTagContainer AbilityTagContainer;
		AbilityTagContainer.AddTag(ActivateTagName);
		Enemy->GetAbilitySystemComponent()->TryActivateAbilitiesByTag(AbilityTagContainer);
	}
	
	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UBTTask_ActivateAbility::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_ActivateAbility::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                 EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
