// Fill out your copyright notice in the Description page of Project Settings.


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
	// Target의 Health를 Capture (최종 데미지 적용용)
	DECLARE_ATTRIBUTE_CAPTUREDEF(Damage);

	FDamageStatics()
	{
		// UMyAttributeSet의 Defensive, Target에서, Snapshot 안 함 (실시간 값)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKOCombatSet, Defense, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKOHealthSet, Damage, Target, false);
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
	RelevantAttributesToCapture.Add(DamageStatics().DamageDef);
}

void UKOExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                                FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	if (!SourceASC || !TargetASC) return;

	AActor* SourceActor = SourceASC->GetAvatarActor();
	AActor* TargetActor = TargetASC->GetAvatarActor();
	if (!SourceActor || !TargetActor)
	{
		return;
	}


	float RawDamage = Spec.GetSetByCallerMagnitude(
		KOGameplayTags::Data_Damage,
		false,
		30.0f
	);

	float Defense = 0.f;

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
		DamageStatics().DefenseDef,
		EvalParams,
		Defense
	);


	float FinalDamage = RawDamage / (1.00f + (Defense * 0.01f));
	FinalDamage = FMath::Max(FinalDamage, 0.f);


	if (FinalDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				DamageStatics().DamageProperty,
				EGameplayModOp::Additive,
				FinalDamage
			)
		);

		//맞은게 Enemy라면, 피격 퍼셉션 자극
		if (AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(TargetActor))
		{
			UAISense_Damage::ReportDamageEvent(
				Enemy->GetWorld(),
				Enemy,
				SourceActor, 
				FinalDamage,
				SourceActor->GetActorLocation(),
				Enemy->GetActorLocation()
			);
		}
	}
}
