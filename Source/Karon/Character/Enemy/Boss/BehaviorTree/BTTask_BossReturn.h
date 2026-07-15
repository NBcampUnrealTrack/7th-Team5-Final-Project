#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_BossReturn.generated.h"

UCLASS()
class KARON_API UBTTask_BossReturn : public UBTTaskNode
{
	GENERATED_BODY()
 
public:
	UBTTask_BossReturn();
 
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;
 
	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTNodeResult::Type TaskResult
	) override;
 
private:
	UPROPERTY(EditAnywhere, Category = "Return")
	float AcceptanceRadius = 100.f;
	
	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result);
 
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};
