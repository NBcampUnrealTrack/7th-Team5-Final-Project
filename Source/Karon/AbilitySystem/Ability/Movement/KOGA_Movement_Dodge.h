#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/KOGameplayAbilityBase.h"
#include "KOGA_Movement_Dodge.generated.h"

UENUM(BlueprintType)
enum class EDodgeDirection : uint8
{
	Forward,
	ForwardRight,
	Right,
	BackRight,
	Backward,
	BackLeft,
	Left,
	ForwardLeft,
};

// TODO: 
// Montage Curve로 속도 조절


UCLASS()
class KARON_API UKOGA_Movement_Dodge : public UKOGameplayAbilityBase
{
	GENERATED_BODY()
	
public:
	UKOGA_Movement_Dodge();

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
	
protected:
	
	FVector CalculateDodgeDirection() const;
	
	EDodgeDirection ClassifyDirection(FVector Forward, FVector InputDir) const;
	
private:
	UFUNCTION() 
	void OnMontageCompleted();
	
	UFUNCTION() 
	void OnMontageCancelled();
	
	UFUNCTION()
	void OnStartEventReceived(FGameplayEventData Data);
	
	UFUNCTION()
	void OnEndEventReceived(FGameplayEventData Data);
	
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Montage")
	TMap<EDodgeDirection, UAnimMontage*> DodgeMontages;
	
	UPROPERTY(EditDefaultsOnly, Category = "Invincible")
	TSubclassOf<UGameplayEffect> GE_Invincible;
	
	UPROPERTY()
	FActiveGameplayEffectHandle GE_InvincibleHandle; 
	
	UPROPERTY(EditDefaultsOnly, Category = "Launch")
	float LaunchDistance = 400.f;
};
