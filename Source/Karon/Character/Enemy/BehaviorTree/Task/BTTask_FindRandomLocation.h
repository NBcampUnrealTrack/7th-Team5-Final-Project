#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_FindRandomLocation.generated.h"

/**
 * NavMesh 내에서 랜덤한 위치를 찾아 PatrolLocation에 반환합니다.
 */
UCLASS()
class KARON_API UBTTask_FindRandomLocation : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_FindRandomLocation();
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolLocationKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector InitialLocationKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector MaxDistanceKey;
};
