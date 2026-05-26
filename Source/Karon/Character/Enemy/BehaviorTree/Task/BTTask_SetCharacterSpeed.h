// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_SetCharacterSpeed.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UBTTask_SetCharacterSpeed : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_SetCharacterSpeed();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="BehaviorTree|BBKey")
	FBlackboardKeySelector SelfActorKey;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="BehaviorTree|BBKey")
	FBlackboardKeySelector EnemySpeed;
	
};
