#pragma once

#include "CoreMinimal.h"
#include "KOGA_BossAttackBase.h"

#include "KOGA_BossAOEAttackBase.generated.h"

class UAbilityTask_WaitGameplayEvent;

UCLASS()
class KARON_API UKOGA_BossAOEAttackBase : public UKOGA_BossAttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_BossAOEAttackBase();
 
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
	
	virtual void OnMontageCompleted() override;
 
protected:
	// 충격파 반경
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack | AoE")
	float ShockwaveRadius = 500.f;
 
	// 충격파 횟수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack | AoE")
	int32 ShockwaveCount = 3;
 
	// 충격파 간격
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack | AoE")
	float ShockwaveInterval = 0.3f;
	
private:
	// 애님 노티파이 이벤트 수신 시 충격파 시작
	UFUNCTION()
	void OnShockwaveNotify(FGameplayEventData EventData);
 
	// 충격파 실행
	void TriggerShockwave();
 
	int32 CurrentShockwaveCount = 0;
	FTimerHandle ShockwaveTimerHandle;
 
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitEventTask;
};
