#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RotateToTarget.generated.h"

UCLASS()
class KARON_API UBTTask_RotateToTarget : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_RotateToTarget();
 
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;
 
	virtual void TickTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds
	) override;
 
protected:
	UPROPERTY(EditAnywhere, Category = "Rotate")
	float AngleTolerance = 5.f;
	
	UPROPERTY(EditAnywhere, Category = "Rotate")
	float RotateSpeed = 180.f;
};
