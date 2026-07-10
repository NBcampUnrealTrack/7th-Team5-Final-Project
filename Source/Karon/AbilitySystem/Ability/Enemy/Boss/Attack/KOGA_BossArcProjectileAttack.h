#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/Enemy/Boss/Attack/KOGA_BossAttackBase.h"
#include "KOGA_BossArcProjectileAttack.generated.h"

class UAbilityTask_WaitGameplayEvent;
class AKOBossArcProjectile;

UCLASS()
class KARON_API UKOGA_BossArcProjectileAttack : public UKOGA_BossAttackBase
{
	GENERATED_BODY()
 
public:
	UKOGA_BossArcProjectileAttack();
 
protected:
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
	virtual void OnMontageCancelled() override;
 
private:
	UPROPERTY(EditAnywhere, Category = "ArcProjectile")
	TSubclassOf<AKOBossArcProjectile> ProjectileClass;
 
	// 사출 개수
	UPROPERTY(EditAnywhere, Category = "ArcProjectile", meta = (ClampMin = "1"))
	int32 ProjectileCount = 6;
 
	// 사출 속도
	UPROPERTY(EditAnywhere, Category = "ArcProjectile")
	float LaunchSpeedXY = 700.f;
 
	// 사출 높이
	UPROPERTY(EditAnywhere, Category = "ArcProjectile")
	float LaunchSpeedZ = 600.f;
 
	// 투사체 간 간격 (0이면 동시 발사)
	UPROPERTY(EditAnywhere, Category = "ArcProjectile", meta = (ClampMin = "0.0"))
	float SpawnInterval = 0.1f;
	
	UPROPERTY(EditAnywhere, Category = "ArcProjectile", meta = (ClampMin = "0.0"))
	float EndDelay = 1.0f;
	
	// true : 매번 다른 각도로 사출
	// false : 매번 동일한 각도로 사출
	UPROPERTY(EditAnywhere, Category = "ArcProjectile")
	bool bRandomStartAnglePerBurst = true;

	UPROPERTY(EditAnywhere, Category = "ArcProjectile", meta = (ClampMin = "1"))
	int32 BurstCount = 1;

	UPROPERTY(EditAnywhere, Category = "ArcProjectile", meta = (ClampMin = "0.0"))
	float BurstInterval = 0.8f;

	UPROPERTY(EditAnywhere, Category = "ArcProjectile")
	FGameplayTag LaunchEventTag;
	
	int32 SpawnedCount = 0;
	int32 CurrentBurst = 0;
	float BurstStartAngle = 0.f;
	FTimerHandle SpawnTimerHandle;
	FTimerHandle BurstTimerHandle;
	FTimerHandle EndDelayTimerHandle;
 
	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitLaunchEventTask;
 
	UFUNCTION()
	void OnLaunchEventReceived(FGameplayEventData Payload);
	
	void StartBurst();
	
	void SpawnOneProjectile();
	
	void OnAllBurstsCompleted();

	void StopMontageAndEnd();
};
