#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "KOAIC_BossChapter01.generated.h"

UCLASS()
class KARON_API AKOAIC_BossChapter01 : public AAIController
{
	GENERATED_BODY()
	
public:
	AKOAIC_BossChapter01();
 
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
 
protected:
	// BP에서 BT 에셋 연결
	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;
 
public:
	static const FName TargetActorKey;
	static const FName bIsInTraceRangeKey;
	static const FName bIsPhase2Key;
	static const FName bIsGroggyKey;
	static const FName bIsAttackingKey;
	static const FName bIsDeadKey;
	static const FName GroundHitDirCheckKey;
	
	// 기믹 돌진 준비
	static const FName bIsGimmickReadyKey;
	// 기믹 돌진 목표 위치
	static const FName GimmickLocationKey;
};
