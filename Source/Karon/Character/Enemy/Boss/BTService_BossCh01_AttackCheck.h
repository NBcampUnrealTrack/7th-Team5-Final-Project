#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_BossCh01_AttackCheck.generated.h"

USTRUCT()
struct FBossAttackInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	FGameplayTag CooldownTag;

	UPROPERTY(EditAnywhere)
	FName BBKey;

	UPROPERTY(EditAnywhere)
	float AttackRange = 0.f;
};

UCLASS()
class KARON_API UBTService_BossCh01_AttackCheck : public UBTService
{
	GENERATED_BODY()
	
public:
	UBTService_BossCh01_AttackCheck();
 
protected:
	virtual void TickNode(
		UBehaviorTreeComponent& OwnerComp,
		uint8* NodeMemory,
		float DeltaSeconds
	) override;
	
	UPROPERTY(EditAnywhere, Category = "Detection")
	TArray<FBossAttackInfo> AttackInfos;
};
