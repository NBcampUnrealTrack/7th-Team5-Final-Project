// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "BTDecorator_CheckKeyTrue.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UBTDecorator_CheckKeyTrue : public UBTDecorator_Blackboard
{
	GENERATED_BODY()
public:
	UBTDecorator_CheckKeyTrue();

public:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
	virtual EBlackboardNotificationResult OnBlackboardKeyValueChange(const UBlackboardComponent& Blackboard, FBlackboard::FKey KeyID) override;
};
