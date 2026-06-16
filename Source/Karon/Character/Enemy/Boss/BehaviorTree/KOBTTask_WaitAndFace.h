#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "KOBTTask_WaitAndFace.generated.h"

UCLASS()
class KARON_API UKOBTTask_WaitAndFace : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UKOBTTask_WaitAndFace();
 
	virtual EBTNodeResult::Type ExecuteTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory
	) override;
 
	virtual void TickTask(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds
	) override;
 
	virtual void OnTaskFinished(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		EBTNodeResult::Type TaskResult
	) override;
 
protected:
	// 응시 타겟
	UPROPERTY(EditAnywhere, Category = "Face")
	FBlackboardKeySelector FaceTargetKey;
	
	// 타겟 응시 대기 시간
	UPROPERTY(EditAnywhere, Category = "Wait")
	float WaitDuration = 2.f;
 
	// 응시 회전 속도
	UPROPERTY(EditAnywhere, Category = "Wait")
	float RotateSpeed = 180.f;
 
private:
	FTimerHandle WaitTimerHandle;
	bool bWaitFinished = false;
};
