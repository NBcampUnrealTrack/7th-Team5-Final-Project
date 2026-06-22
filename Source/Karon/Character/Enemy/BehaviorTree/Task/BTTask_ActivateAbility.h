#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ActivateAbility.generated.h"

class UAbilitySystemComponent;
/**
 * 해당 태그를 가진 어빌리티 중에서 랜덤으로 실행합니다.
 */
UCLASS()
class KARON_API UBTTask_ActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UBTTask_ActivateAbility();
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	UFUNCTION()
	void OnSkillTagRemoved(const FGameplayTag Tag, int32 NewCount, UBehaviorTreeComponent* OwnerComp);
	
protected:
	//옵저버 어봇: 중단될 시 호출되는 함수. 
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	// 성공/실패/중단, 아무튼 무관하게 종료될 시
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	
private:
	UAbilitySystemComponent* GetASC(UBehaviorTreeComponent& OwnerComp);
	
protected:
	UPROPERTY(EditAnywhere, Category = "AnimationMontage")
	FGameplayTag ActivateTagName;
	
	//명시적으로 죽음 GA는 어보트되지 않도록 한다.
	UPROPERTY()
	FGameplayTag DeathTag;
	
	//몽타주가 멈추기까지 블렌드되는 시간
	float MontageBlendOutTime=0.05f;
	
};
