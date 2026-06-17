#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Ability/Attack/KOGA_Charge_Attack_Base.h"
#include "KOGA_Skill_DimensionSever.generated.h"

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;

UCLASS()
class KARON_API UKOGA_Skill_DimensionSever : public UKOGA_Charge_Attack_Base
{
	GENERATED_BODY()
	
public:
	UKOGA_Skill_DimensionSever();
	
protected:
	virtual void ExecuteAttack(float ChargePercentage) override;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Skill|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;
	
private:
	UFUNCTION()
	void OnAttackMontageEnded();
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> AttackMontageTask;
};
