#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/Enemy/Boss/Attack/KOGA_BossAttackBase.h"
#include "KOGA_BossJumpAttack.generated.h"

class UAbilityTask_WaitGameplayEvent;

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
	
	virtual void OnMontageCompleted() override;
	virtual void OnMontageCancelled() override;

private:
	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	float JumpHeight = 900.f;

	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	float JumpTime = 1.4f;

	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	float HomingStrength = 0.2f;
	
	UPROPERTY(EditAnywhere, Category = "JumpAttack|Gravity", meta = (ClampMin = "0.1"))
	float RiseGravityMultiplier = 1.3f;
	
	UPROPERTY(EditAnywhere, Category = "JumpAttack|Gravity", meta = (ClampMin = "0.05"))
	float FloatGravityMultiplier = 0.15f;

	UPROPERTY(EditAnywhere, Category = "JumpAttack|Gravity", meta = (ClampMin = "1.0"))
	float FallGravityMultiplier = 3.2f;
	
	UPROPERTY(EditAnywhere, Category = "JumpAttack|Gravity", meta = (ClampMin = "0.0"))
	float FloatHoldTime = 0.5f;
	
	UPROPERTY(EditAnywhere, Category = "JumpAttack|Gravity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RiseHorizontalRatio = 0.3f;
	
	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	FGameplayTag LaunchEventTag;

	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	FName JumpSection  = FName("Jump");

	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	FName FloatSection = FName("Float");

	UPROPERTY(EditAnywhere, Category = "JumpAttack")
	FName LandSection  = FName("Land");
	
	FVector LaunchTargetLocation = FVector::ZeroVector;
	FTimerHandle HomingTimerHandle;
	
	enum class EJumpGravityPhase : uint8
	{
		Rising,
		Floating,
		Falling
	};
	EJumpGravityPhase CurrentGravityPhase = EJumpGravityPhase::Rising;
	
	float BaseGravity = 0.f;
	
	float BaseHorizontalSpeed = 0.f;

	FTimerHandle FloatHoldTimerHandle;

	void StartFalling();

	UPROPERTY()
	TObjectPtr<UAbilityTask_WaitGameplayEvent> WaitLaunchEventTask;

	void Launch();
	
	UFUNCTION()
	void OnLaunchEventReceived(FGameplayEventData Payload);

	UFUNCTION()
	void UpdateHoming();

	UFUNCTION()
	void OnCharacterLanded(const FHitResult& Hit);

	void BindLandedDelegate();
	void UnbindLandedDelegate();
	
	void AddPlayerToMoveIgnore();
	void RemovePlayerFromMoveIgnore();
};
