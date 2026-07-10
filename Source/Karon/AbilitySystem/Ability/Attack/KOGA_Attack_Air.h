#pragma once

#include "CoreMinimal.h"
#include "KOGA_AttackBase.h"
#include "KOGA_Attack_Air.generated.h"

class UCharacterMovementComponent;
UCLASS()
class KARON_API UKOGA_Attack_Air : public UKOGA_AttackBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Attack_Air();
	
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
	)override;
	
protected:
	UFUNCTION()
	void OnTraceStart(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnTraceEnd(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnGroundImpact(FGameplayEventData Payload);
	
	UFUNCTION()
	void OnMontageTaskCancelled();
	
	UFUNCTION()
	void OnMontageTaskCompleted();
	
	UFUNCTION()
	void OnTick(float DeltaTime);
	
private:
	void HandleLandedState(int32 MaxSectionIndex);
	
	bool FindGroundImpactPoint(FVector& OutImpactPoint, FHitResult& OutGroundHit);
	
	void ApplyRadialDamage(const FVector& ImpactPoint, const FHitResult& GroundHit);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TArray<FName> MontageSectionNames;
	
	int32 CurrentSectionIndex = 0; 
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float AttackRadius = 300.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float DownwardForce = 1800.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Cue")
	FGameplayTag GroundImpactCueTag; 
	
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC; 
	
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> MovementComponent = nullptr;
	
	UPROPERTY()
	TObjectPtr<class UAbilityTask_Tick> CheckFallTask;
};



