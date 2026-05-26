#include "Character/Enemy/Boss/KOBTTask_ActivateBossAbility.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AIController.h"

UKOBTTask_ActivateBossAbility::UKOBTTask_ActivateBossAbility()
{
	NodeName = TEXT("Activate Boss Ability");
 
	// GA 종료 알림을 받기 위해 true
	bNotifyTaskFinished = true;
}
 
EBTNodeResult::Type UKOBTTask_ActivateBossAbility::ExecuteTask(
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
	
	FGameplayTagContainer ActiveTags;
	ASC->GetOwnedGameplayTags(ActiveTags);
	
	// 태그로 GA 발동
	bool bSuccess = ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AbilityTag));
 
	if (!bSuccess)
	{
		return EBTNodeResult::Failed;
	}
 
	// GA 종료까지 BT 대기
	return EBTNodeResult::InProgress;
}
 
void UKOBTTask_ActivateBossAbility::OnTaskFinished(
	UBehaviorTreeComponent& OwnerComp,
	uint8* NodeMemory,
	EBTNodeResult::Type TaskResult)
{
	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}
