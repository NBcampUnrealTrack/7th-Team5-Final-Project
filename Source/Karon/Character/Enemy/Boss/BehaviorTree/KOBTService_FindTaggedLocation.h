#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "KOBTService_FindTaggedLocation.generated.h"

UCLASS()
class KARON_API UKOBTService_FindTaggedLocation : public UBTService
{
	GENERATED_BODY()
	
public:
	UKOBTService_FindTaggedLocation();
 
protected:
	virtual void TickNode(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds
	) override;
	
	UPROPERTY(EditAnywhere, Category = "Location")
	FName SearchActorTag;
	
	UPROPERTY(EditAnywhere, Category = "Location")
	FBlackboardKeySelector LocationKey;
	
private:
	bool bIsLocationFound = false;
};
