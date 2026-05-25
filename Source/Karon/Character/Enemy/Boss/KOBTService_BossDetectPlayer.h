#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "KOBTService_BossDetectPlayer.generated.h"

UCLASS()
class KARON_API UKOBTService_BossDetectPlayer : public UBTService
{
	GENERATED_BODY()
	
public:
	UKOBTService_BossDetectPlayer();
	
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, Category="Detection")
	float TraceRange = 1500.f;
	
	UPROPERTY(EditAnywhere, Category="Detection")
	float AttackRange = 500.f;
};
