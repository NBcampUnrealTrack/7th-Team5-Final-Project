#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "KOAIC_Boss.generated.h"

UCLASS()
class KARON_API AKOAIC_Boss : public AAIController
{
	GENERATED_BODY()
	
public:
	AKOAIC_Boss();
 
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
 
protected:
	// BP에서 BT 에셋 연결
	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;
 
public:
	static const FName TargetActorKey;		 // 추적 대상
	static const FName bIsInTraceRangeKey;	 // 추적 범위 안
	static const FName bIsInAttackRangeKey;	 // 공격 범위 안
	static const FName bIsPhase2Key;		 // 페이즈2 여부
	static const FName bIsGroggyKey;		 // 그로기 여부
};
