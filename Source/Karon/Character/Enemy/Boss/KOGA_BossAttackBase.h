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
	
protected:
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Attack | Montage")
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Attack | Montage")
	float MontageSpeed = 1.f;

	UFUNCTION()
	virtual void OnMontageCompleted();
	UFUNCTION()
	virtual void OnMontageCancelled();
};
