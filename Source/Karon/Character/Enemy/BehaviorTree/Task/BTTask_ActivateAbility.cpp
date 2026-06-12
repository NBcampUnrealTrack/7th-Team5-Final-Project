// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ActivateAbility.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "GameFramework/Character.h"


UBTTask_ActivateAbility::UBTTask_ActivateAbility()
{
	bNotifyTick = false;
	DeathTag=KOGameplayTags::State_Enemy_Dead;
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
	AAIController* Owner = OwnerComp.GetAIOwner();
	if (Owner == nullptr)
	{
		return EBTNodeResult::Aborted;
	}
	ACharacter* AICharacter = Cast<ACharacter>(Owner->GetPawn());
	//AIChaarcter가 nullptr이거나, 태그가 비었거나, 죽음 GA를 활성화하는 태그라면 Abort하지 않음.
	if (AICharacter == nullptr || 
		ActivateTagName == FGameplayTag::EmptyTag||
		ActivateTagName==DeathTag)
	{
		return EBTNodeResult::Failed;
	}
	AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(AICharacter);
	if(Enemy==nullptr)
	{
		return EBTNodeResult::Failed;
	}
	//Abort시 적용중인 GA 캔슬
	if (Enemy->GetAbilitySystemComponent())
	{
		FGameplayTagContainer AbilityTagContainer;
		AbilityTagContainer.AddTag(ActivateTagName);
		Enemy->GetAbilitySystemComponent()->CancelAbilities(&AbilityTagContainer);
	}
	//애님 몽타주도 캔슬
	if (UAnimInstance* AnimInstance = Enemy->GetMesh()->GetAnimInstance())
	{
		if (AnimInstance->IsAnyMontagePlaying())
		{
			AnimInstance->Montage_Stop(MontageBlendOutTime); 
		}
	}
	UE_LOG(LogTemp,Warning,TEXT("Aborted"));
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_ActivateAbility::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                                 EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
