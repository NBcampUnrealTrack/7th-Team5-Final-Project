#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_HitReaction.generated.h"


UCLASS()
class KARON_API UKOGA_HitReaction : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_HitReaction(); 
	
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
	
	// 피격 감지 
	// 방향 분기 
	// GE (데미지 / 상태이상 등)  + 애니메이션
	// 히트 스톱 Task ? 
	// 넉백 
	// 카메라 쉐이크  + VFX  << 이건 Cue ? 
	// 짧은 무적 후 종료 
protected:
	UAnimMontage* SelectMontageByDirection(const FVector& HitDirection) const;
	
	FGameplayTag CalcDirectionTag(const FVector& HitDirection) const;

private:
	UFUNCTION()
	void OnHitStopFinished();

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageCancelled();
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Montage")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> DirectionalMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KnockBack")
	float KnockBackAmount; 
};
