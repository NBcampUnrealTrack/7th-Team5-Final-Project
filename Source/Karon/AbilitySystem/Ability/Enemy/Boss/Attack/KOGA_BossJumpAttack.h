#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/Enemy/Boss/Attack/KOGA_BossAttackBase.h"
#include "KOGA_BossJumpAttack.generated.h"

UCLASS()
class KARON_API UKOGA_BossJumpAttack : public UKOGA_BossAttackBase
{
	GENERATED_BODY()

public:
	UKOGA_BossJumpAttack();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

private:
	// 점프 파라미터
	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	float JumpHeight = 600.f;

	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	float JumpTime = 1.2f;

	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	float HomingStrength = 0.2f;

	// 몽타주 섹션
	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	FName JumpSection  = FName("Jump");

	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	FName FloatSection = FName("Float");

	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	FName LandSection  = FName("Land");
	
	FVector LaunchTargetLocation = FVector::ZeroVector;
	FTimerHandle HomingTimerHandle;

	void Launch();

	UFUNCTION()
	void UpdateHoming();

	UFUNCTION()
	void OnCharacterLanded(const FHitResult& Hit);

	void BindLandedDelegate();
	void UnbindLandedDelegate();
	
};
