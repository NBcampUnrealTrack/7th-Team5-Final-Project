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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly,Category="Attack | State")
	float AttackRange = 300.f;

protected:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Attack | Montage")
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Attack | Montage")
	float MontageSpeed = 1.f;

	UFUNCTION()
	virtual void OnMontageCompleted();
	UFUNCTION()
	virtual void OnMontageCancelled();

	bool IsTargetInRange() const;
	
private:
	UPROPERTY()
	TObjectPtr<class UAbilityTask_PlayMontageAndWait> MontageTask;
};
