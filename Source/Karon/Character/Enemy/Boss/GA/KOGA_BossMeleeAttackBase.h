#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Boss/KOGA_BossAttackBase.h"
#include "KOGA_BossMeleeAttackBase.generated.h"

class UAbilityTask_WaitGameplayEvent;

UCLASS()
class KARON_API UKOGA_BossMeleeAttackBase : public UKOGA_BossAttackBase
{
	GENERATED_BODY()

public:
	UKOGA_BossMeleeAttackBase();
	
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
 
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;
	
	 
	// NotifyState에서 소켓 위치 가져올 때 사용
	FVector GetAttackSocketLocation() const;
	
	// 공격에 사용할 소켓 이름 (BP에서 공격마다 설정)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack | Socket")
	FName AttackSocketName = FName("hand_r");
	
private:
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitHitEventTask;
 
	UFUNCTION()
	void OnHitEventReceived(FGameplayEventData EventData);
};
