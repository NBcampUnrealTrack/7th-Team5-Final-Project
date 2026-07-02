#include "KOExecCalc_Damage.h"

#include "KOGameplayEffectContext.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "AbilitySystem/Attribute/KOGuardSet.h"
#include "AbilitySystem/Attribute/KOHealthSet.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Utility/Log/KOLogManager.h"


struct FDamageStatics
{
	// Target의 Defensive를 Capture
	DECLARE_ATTRIBUTE_CAPTUREDEF(Defense);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CritMultiplier);
	DECLARE_ATTRIBUTE_CAPTUREDEF(GuardHealth);
	
	FDamageStatics()
	{
		// UMyAttributeSet의 Defensive, Target에서, Snapshot 안 함 (실시간 값)
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKOCombatSet, Defense,         Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKOCombatSet, CritChance,      Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKOCombatSet, CritMultiplier,  Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UKOGuardSet,  GuardHealth,     Target, false);
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
	RelevantAttributesToCapture.Add(DamageStatics().GuardHealthDef);
}

void UKOExecCalc_Damage::Execute_Implementation(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput
	) const
{
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	if (!TargetASC || !SourceASC) return;
	
	if (TargetASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_Dead))
		return; 

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvalParams;
	EvalParams.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvalParams.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	const float FinalDamage = CalculateFinalDamage(ExecutionParams, EvalParams, Spec);
	if (FinalDamage <= 0.f) return;

	RouteGuardDamage(FinalDamage, SourceASC, TargetASC, EvalParams, ExecutionParams, OutExecutionOutput);
}

float UKOExecCalc_Damage::CalculateFinalDamage(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	const FAggregatorEvaluateParameters& EvalParams, 
	const FGameplayEffectSpec& Spec) const
{
	// 1. Raw Damage From SetByCaller
	const float RawDamage = Spec.GetSetByCallerMagnitude(KOGameplayTags::Data_Damage, false, 0.f);
	
	// 2. Get Attributes from Capture 
	float Defense = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefenseDef, EvalParams, Defense);
	
	float CritChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritChanceDef, EvalParams, CritChance);

	float CritMultiplier = 1.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CritMultiplierDef, EvalParams, CritMultiplier);
	
	// 3. Calculate Final Damage
	float FinalDamage = FMath::Max(RawDamage / (1.f + Defense * 0.01f), 0.f);

	// 4. Judge Critical 
	const bool bIsCritical = FMath::FRand() < CritChance;
	if (bIsCritical) FinalDamage *= CritMultiplier; 
	
	FString CritStr = bIsCritical ? TEXT(" (Critical!)") : TEXT("");
	KO_LOG(Combat, Log, TEXT("Final Damage : %f%s"), FinalDamage, *CritStr);
	
	if (FKOGameplayEffectContext* KOContext = static_cast<FKOGameplayEffectContext*>(Spec.GetContext().Get()))
	{
		KOContext->SetIsCriticalHit(bIsCritical);
	}
	
	return FinalDamage;
}

void UKOExecCalc_Damage::RouteGuardDamage(
	float FinalDamage,
	UAbilitySystemComponent* SourceASC,
	UAbilitySystemComponent* TargetASC,
	const FAggregatorEvaluateParameters& EvalParams,
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	float HealthDamage = FinalDamage;

	if (TargetASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_Guard_Blocking))
	{
		// 방향 체크
		AActor* TargetActor = TargetASC->GetAvatarActor();
		AActor* SourceActor = ExecutionParams.GetSourceAbilitySystemComponent()->GetAvatarActor();

		const FVector ToAttacker = 
			(SourceActor->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal();
		
		const float Dot = FVector::DotProduct(TargetActor->GetActorForwardVector(), ToAttacker);

		if (Dot > 0.f)
		{
			// 정면 → Guard 흡수
			float CurrentGuardHealth = 0.f;
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
				DamageStatics().GuardHealthDef, EvalParams, CurrentGuardHealth);

			const float GuardDamage = FMath::Min(FinalDamage, CurrentGuardHealth);
			HealthDamage -= GuardDamage;

			KO_LOG(Combat, Log, TEXT("Block Damage : %f"), GuardDamage);

			OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
				UKOGuardSet::GetGuardDamageAttribute(), EGameplayModOp::Additive, GuardDamage));
			
			FGameplayEventData EventData;
			TargetASC->HandleGameplayEvent(KOGameplayTags::Event_Guard_Success, &EventData);
			
			//방향이 맞는 상태에서 퍼펙트가드 성공시 해당 에너미 그로기
			if (TargetASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_Guard_PerfectGuard))
			{
				FGameplayEventData ParryEventData;
				SourceASC->HandleGameplayEvent(KOGameplayTags::Event_Parried, &ParryEventData);
			}
			
		}
		else
		{
			FGameplayEventData EventData;
			TargetASC->HandleGameplayEvent(KOGameplayTags::Event_Guard_DirectionFail, &EventData);
		}
	}

	if (HealthDamage > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
			UKOHealthSet::GetDamageAttribute(), EGameplayModOp::Additive, HealthDamage));
		
		FGameplayEventData EventData;
		EventData.EventMagnitude = HealthDamage;
		
		TargetASC->HandleGameplayEvent(KOGameplayTags::Event_HitReact, &EventData);
		SourceASC->HandleGameplayEvent(KOGameplayTags::Event_Clock_Gain, &EventData);
	}
}
