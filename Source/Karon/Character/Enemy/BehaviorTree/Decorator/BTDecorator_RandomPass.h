// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "BTDecorator_RandomPass.generated.h"

/**
 * 확률 기반으로 Pass/Fail을 결정하는 데코레이터입니다.
 */
UCLASS()
class KARON_API UBTDecorator_RandomPass : public UBTDecorator_Blackboard
{
	GENERATED_BODY()
public:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
protected:
	UPROPERTY(EditAnywhere)
	float Percent=100.f;
};
