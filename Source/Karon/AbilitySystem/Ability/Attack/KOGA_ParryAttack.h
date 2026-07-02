// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KOGA_AttackBase.h"
#include "KOGA_ParryAttack.generated.h"

class UKOGA_Parried;
class AKOHeroCharacter;
class AKOBaseEnemy;
/**
 * 
 */
UCLASS()
class KARON_API UKOGA_ParryAttack : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_ParryAttack();
	
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
	bool bReplicateEndAbility, bool bWasCancelled
	) override;

	
private:
	UFUNCTION()
	void OnMontageCompleted();
	
	UFUNCTION()
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnNotifyHitEvent(FGameplayEventData HitGameplayEventData);
	
	void ExecuteParryAttack(AKOBaseEnemy* Enemy);

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY(EditAnywhere)
	float Damage=30.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> Montage;
	
private:
	UPROPERTY()
	AKOBaseEnemy* CachedEnemy=nullptr;
	
	UPROPERTY()
	AKOHeroCharacter* CachedPlayer=nullptr;
	
	UPROPERTY()
	UKOGA_Parried* CachedParriedGA;
	
	FName MotionWarpingName=FName("FrontStabTarget");
	
	float MotionWarpingDistance=500.f;
	
	void CharacterRotateLock(bool bIsLocked);
};
