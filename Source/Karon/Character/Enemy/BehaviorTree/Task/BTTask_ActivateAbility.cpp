// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_ActivateAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "AbilitySystem/Tag/State/KOGameplayTags_State.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "GameFramework/Character.h"


UBTTask_ActivateAbility::UBTTask_ActivateAbility()
{
	bNotifyTick = false;
	DeathTag = KOGameplayTags::State_Enemy_Dead;
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
	AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(AICharacter);
	if (Enemy == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = GetASC(OwnerComp);
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	// 중복 시전 방지
	if (ASC->HasMatchingGameplayTag(ActivateTagName))
	{
		return EBTNodeResult::Failed;
	}
	// 해당 태그가 추가/제거시 바인딩
	ASC->RegisterGameplayTagEvent(ActivateTagName, EGameplayTagEventType::NewOrRemoved)
	   .AddUObject(this, &UBTTask_ActivateAbility::OnSkillTagRemoved, &OwnerComp);


	//해당 태그에 매칭되는 GA 중에서 랜덤하게 하나만 실행한다.
	FGameplayTagContainer AbilityTagContainer;
	AbilityTagContainer.AddTag(ActivateTagName);

	TArray<FGameplayAbilitySpec*> ActivatableAbilities;
	ASC->GetActivatableGameplayAbilitySpecsByAllMatchingTags(AbilityTagContainer, ActivatableAbilities);
	if (ActivatableAbilities.IsEmpty())
	{
		return EBTNodeResult::Failed;
	}

	int32 RandomIndex = FMath::RandRange(0, ActivatableAbilities.Num() - 1);
	ASC->TryActivateAbility(ActivatableAbilities[RandomIndex]->Handle);


	return EBTNodeResult::InProgress;
}

void UBTTask_ActivateAbility::OnSkillTagRemoved(const FGameplayTag Tag, int32 NewCount,
                                                UBehaviorTreeComponent* OwnerComp)
{
	// 태그 제거시에만 적용
	if (NewCount != 0)
	{
		return;
	}

	UAbilitySystemComponent* ASC = GetASC(*OwnerComp);
	if (!ASC)
	{
		return;
	}

	//바인딩 제거
	ASC->RegisterGameplayTagEvent(ActivateTagName, EGameplayTagEventType::NewOrRemoved)
	   .RemoveAll(this);

	//태스크 종료
	if (OwnerComp)
	{
		FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
	}
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
		ActivateTagName == FGameplayTag::EmptyTag ||
		ActivateTagName == DeathTag)
	{
		return EBTNodeResult::Failed;
	}
	AKOBaseEnemy* Enemy = Cast<AKOBaseEnemy>(AICharacter);
	if (Enemy == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	UAbilitySystemComponent* ASC = GetASC(OwnerComp);
	if (!ASC)
	{
		return EBTNodeResult::Failed;
	}

	//바인딩 제거
	ASC->RegisterGameplayTagEvent(ActivateTagName, EGameplayTagEventType::NewOrRemoved)
	   .RemoveAll(this);
	
	//Abort시 적용중인 GA 캔슬
	FGameplayTagContainer AbilityTagContainer;
	AbilityTagContainer.AddTag(ActivateTagName);
	Enemy->GetAbilitySystemComponent()->CancelAbilities(&AbilityTagContainer);


	//애님 몽타주도 캔슬
	if (UAnimInstance* AnimInstance = Enemy->GetMesh()->GetAnimInstance())
	{
		if (AnimInstance->IsAnyMontagePlaying())
		{
			AnimInstance->Montage_Stop(MontageBlendOutTime);
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Aborted"));
	return Super::AbortTask(OwnerComp, NodeMemory);
}

void UBTTask_ActivateAbility::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory,
                                             EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

UAbilitySystemComponent* UBTTask_ActivateAbility::GetASC(UBehaviorTreeComponent& OwnerComp)
{
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		if (APawn* Pawn = AIController->GetPawn())
		{
			return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Pawn);
		}
	}
	return nullptr;
}
