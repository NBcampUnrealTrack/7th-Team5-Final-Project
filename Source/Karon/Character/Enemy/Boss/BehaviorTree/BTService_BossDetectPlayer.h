#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_BossDetectPlayer.generated.h"

UCLASS()
class KARON_API UBTService_BossDetectPlayer : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_BossDetectPlayer();
	
protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, Category="Detection")
	float TraceRange = 1500.f;
};
