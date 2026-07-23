#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/Enemy/Boss/Attack/KOGA_BossAttackBase.h"
#include "KOGA_BossDashAttack.generated.h"

UCLASS()
class KARON_API UKOGA_BossDashAttack : public UKOGA_BossAttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_BossDashAttack();
 
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
 
protected:
	// 돌진 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	float DashSpeed = 1200.f;
 
	// 최대 돌진 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	float DashDuration = 1.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	bool bIsGimmickDash = false;
 
	UPROPERTY(EditAnywhere, Instanced, Category = "Dash")
	TObjectPtr<UKO_HitData> HitData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash|SFX")
	TObjectPtr<USoundBase> GimmickSuccessSFX;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dash")
	TObjectPtr<UAnimMontage> PreDashMontage;
	
private:
	FVector DashDirection = FVector::ZeroVector;
 
	FTimerHandle DashTimerHandle;
	FTimerHandle DashVelocityTimerHandle;
	
	FTimerHandle DashHitScanTimerHandle;
	
	TArray<TWeakObjectPtr<AActor>> DashedActors;
	
	UFUNCTION()
	void OnDashHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);
 
	void StopDash();
	void OnDashTimeOut();

	void StartDash();

	UFUNCTION()
	void OnPreDashMontageCompleted();

	UFUNCTION()
	void OnPreDashMontageCancelled();
 
	// 기믹 처리
	void HandleGimmickPillarHit(AActor* PillarActor);
};
