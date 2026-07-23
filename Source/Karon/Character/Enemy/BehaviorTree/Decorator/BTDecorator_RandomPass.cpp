// Fill out your copyright notice in the Description page of Project Settings.


#include "BTDecorator_RandomPass.h"

bool UBTDecorator_RandomPass::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return FMath::RandRange(0.f, 100.f) <= Percent;
}
