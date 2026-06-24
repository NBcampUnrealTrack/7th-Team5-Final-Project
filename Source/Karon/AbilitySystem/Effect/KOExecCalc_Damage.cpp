#include "KOExecCalc_Damage.h"

#include "KOGameplayEffectContext.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Utility/Log/KOLogManager.h"


struct FDamageStatics
{
	// Target의 Defensive를 Capture
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritMultiplier);
	
	FDamageStatics()
	{
		// UMyAttributeSet의 Defensive, Target에서, Snapshot 안 함 (실시간 값)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKOCombatSet, Defense,         Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKOCombatSet, CritChance,      Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKOCombatSet, CritMultiplier,  Source, false);
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
	
	RelevantAttributesToCapture.Add(DamageStatics().CritChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CritMultiplierDef);
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
	
	// 1. Raw Damage From SetByCaller
	float RawDamage = Spec.GetSetByCallerMagnitude(
		KOGameplayTags::Data_Attribute_Health_Damage, false, 0.0f);
	
	// 2. Attribute Capture 
	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	float Defense = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseDef, EvalParams, Defense);
	
	float CritChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritChanceDef, EvalParams, CritChance);

	float CritMultiplier = 1.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritMultiplierDef, EvalParams, CritMultiplier);
	
	// 3. Apply Defense
	float FinalDamage = FMath::Max(RawDamage / (1.f + Defense * 0.01f), 0.f);
	
	// 4. Judge Critical 
	bool bIsCritical = FMath::FRand() < CritChance;
	if (bIsCritical)
	{
		KO_LOG(Combat, Log, TEXT("Critical!!")); 
		FinalDamage *= CritMultiplier;
	}
	
	// 5. Cached Is Critical for GameplayCue 
	if (FKOGameplayEffectContext* KOContext = static_cast<FKOGameplayEffectContext*>(Spec.GetContext().Get()))
	{
		KOContext->SetIsCriticalHit(bIsCritical);
	}
	
	// 6. Final Result 
	if (FinalDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
				UKOHealthSet::GetDamageAttribute(),
				EGameplayModOp::Additive,
				FinalDamage)
		);
	}
}
