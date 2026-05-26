#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "KOBTTask_ActivateBossAbility.generated.h"

UCLASS()
class KARON_API UKOBTTask_ActivateBossAbility : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UKOBTTask_ActivateBossAbility();
 
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;
 
	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTNodeResult::Type TaskResult
	) override;
 
protected:
	// BT 에디터에서 각 공격마다 태그 설정
	UPROPERTY(EditAnywhere, Category = "Ability")
	FGameplayTag AbilityTag;
};
