#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "KOBTTask_MoveToTaggedLocation.generated.h"

namespace EPathFollowingResult
{
	enum Type : int;
}

UCLASS()
class KARON_API UKOBTTask_MoveToTaggedLocation : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UKOBTTask_MoveToTaggedLocation();
 
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
	UPROPERTY(EditAnywhere, Category = "Move")
	FBlackboardKeySelector TargetLocationKey;
 
	UPROPERTY(EditAnywhere, Category = "Move")
	float AcceptanceRadius = 50.f;
 
private:
	UFUNCTION()
	void OnMoveCompleted(
		FAIRequestID RequestID,
		EPathFollowingResult::Type Result
	);
	
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> CachedOwnerComp;
};
