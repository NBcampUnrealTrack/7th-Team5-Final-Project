#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AbilitySystem/Ability/Attack/KOGA_AttackBase.h"
#include "KOGA_BossGroggyBase.generated.h"


UCLASS()
class KARON_API UKOGA_BossGroggyBase : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_BossGroggyBase();
 
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
	// 그로기 유지 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Groggy")
	float GroggyDuration = 5.f;

	// 그로기 + 회복 모션이 들어있는 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Groggy")
	TObjectPtr<UAnimMontage> GroggyMontage;

	// 그로기 루프 섹션 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Groggy")
	FName GroggySection = FName("Groggy");

	// 회복 모션 섹션 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Groggy")
	FName RestoreSection = FName("Restore");
 
private:
	FTimerHandle GroggyTimerHandle;
 
	void OnGroggyTimerEnd();

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();
};
