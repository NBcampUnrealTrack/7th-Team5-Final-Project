#include "KOExecCalc_Damage.h"

#include "AIController.h"
#include "Attribute/KOCombatSet.h"
#include "Attribute/KOHealthSet.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Perception/AISense_Damage.h"
#include "Tag/KOGameplayTags.h"

struct FDamageStatics
{
	// Target의 Defensive를 Capture
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);

	FDamageStatics()
	{
		// UMyAttributeSet의 Defensive, Target에서, Snapshot 안 함 (실시간 값)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKOCombatSet, Defense, Target, false);
	}
};

static const FDamageStatics& DamageStatics()
{
	static FDamageStatics Statics;
	return Statics;
}


UKOExecCalc_Damage::UKOExecCalc_Damage()
{
	// Capture할 Attribute 등록
	RelevantAttributesToCapture.Add(DamageStatics().DefenseDef);
}

void UKOExecCalc_Damage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput
	) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	UAbilitySystemComponent* InstigatorASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	if (!InstigatorASC || !TargetASC) return;
	
	float RawDamage = Spec.GetSetByCallerMagnitude(
	   KOGameplayTags::Data_Attribute_Health_Damage, false, 0.0f);
	
	float Defense = 0.f;
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
	   DamageStatics().DefenseDef, EvalParams, Defense);
	
	float FinalDamage = FMath::Max(RawDamage / (1.f + Defense * 0.01f), 0.f);
	
	if (FinalDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				UKOHealthSet::GetDamageAttribute(),
				EGameplayModOp::Additive,
				FinalDamage));
	}
	
}
