#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "KOGA_BossAttackBase.generated.h"

UCLASS()
class KARON_API UKOGA_BossAttackBase : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UKOGA_BossAttackBase();
 
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
 
	virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags,
		const FGameplayTagContainer* TargetTags,
		FGameplayTagContainer* OptionalRelevantTags
	) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack | Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack | Montage")
	TObjectPtr<UAnimMontage> AttackMontage;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack | Montage")
	float MontageSpeed = 1.f;
 
	UFUNCTION()
	virtual void OnMontageCompleted();
 
	UFUNCTION()
	virtual void OnMontageCancelled();
 
	bool IsTargetInRange() const;
	
	// 공통 데미지 적용 함수
	void ApplyDamageToTarget(AActor* TargetActor);
	
private:
	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> MontageTask;
};
