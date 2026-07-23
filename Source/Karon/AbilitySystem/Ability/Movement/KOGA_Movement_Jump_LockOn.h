#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"

#include "KOGA_Movement_Jump_LockOn.generated.h"

UENUM(BlueprintType)
enum class EJumpDirection : uint8
{
	Idle,
	Forward,
	Backward,
	Left,
	Right
};

UCLASS()
class KARON_API UKOGA_Movement_Jump_LockOn : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Movement_Jump_LockOn();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
private:
	UFUNCTION()
	void OnCharacterLanded(EMovementMode NewMovementMode);

	EJumpDirection DetermineJumpDirection(ACharacter* Character) const;
	
private:
	UPROPERTY(EditDefaultsOnly, Category = "Jump|Montage")
	TMap<EJumpDirection, TObjectPtr<UAnimMontage>> JumpMontages;
	
	UPROPERTY(EditDefaultsOnly, Category = "Jump|Threshold")
	float ForwardThreshold = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Jump|Threshold")
	float IdleSpeedThreshold = 50.f;
};
