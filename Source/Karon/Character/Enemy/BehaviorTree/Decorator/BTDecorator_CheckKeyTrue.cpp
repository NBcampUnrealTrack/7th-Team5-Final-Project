// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_CheckKeyTrue.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_CheckKeyTrue::UBTDecorator_CheckKeyTrue()
{
	BlackboardKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTDecorator_CheckKeyTrue, BlackboardKey));
}

bool UBTDecorator_CheckKeyTrue::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return true;
}

EBlackboardNotificationResult UBTDecorator_CheckKeyTrue::OnBlackboardKeyValueChange(
	const UBlackboardComponent& Blackboard, FBlackboard::FKey KeyID)
{
	// 감시하는 키가 맞는지 확인
	if (KeyID != BlackboardKey.GetSelectedKeyID())
	{
		return EBlackboardNotificationResult::ContinueObserving;
	}
	
	// 값이 true로 변경되었을 때만 Abort 로직을 트리거
	if (Blackboard.GetValueAsBool(GetSelectedBlackboardKey()) == true)
	{
		return Super::OnBlackboardKeyValueChange(Blackboard, KeyID);
	}

	return EBlackboardNotificationResult::ContinueObserving;
}

