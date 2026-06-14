#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_SnowBall.generated.h"


UCLASS()
class KARON_API UKOGA_SnowBall : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_SnowBall();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual void InputReleased(const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo) override;

	UFUNCTION()
	void OnSnowballTick(float DeltaTime);

	UFUNCTION()
	void OnSnowballHit(AActor* HitActor);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Snowball")
	TSubclassOf<AActor> SnowballClass;

	UPROPERTY(EditDefaultsOnly, Category = "Snowball")
	TObjectPtr<class UInputAction> SteerAction;
	
	UPROPERTY(EditDefaultsOnly, Category = "Snowball")
	float GrowthRate = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Snowball")
	float MaxScale = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Snowball")
	float BaseRadius = 50.f;

	UPROPERTY(EditDefaultsOnly, Category = "Snowball")
	float BaseDamage = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Snowball")
	float SteerSpeed = 90.f;

	UPROPERTY(EditDefaultsOnly, Category = "Snowball")
	float BaseSpeedBonus = 100.f;
	
	UPROPERTY()
	TObjectPtr<AActor> SnowballActor;

	float CurrentScale = 1.f;
	float OriginalSpeed = 0.f;
	float SteerInput = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Snowball")
	float Duration = 10.f;
};

