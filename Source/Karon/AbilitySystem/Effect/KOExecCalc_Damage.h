// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "KOExecCalc_Damage.generated.h"

/**
 * GameplayEffect의 데미지를 방어력을 고려하여 계산하는 Execution Calculation Class
 */
UCLASS()
class KARON_API UKOExecCalc_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UKOExecCalc_Damage();

	virtual void Execute_Implementation(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput
	) const override;
	
	float CalculateFinalDamage(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		const FAggregatorEvaluateParameters& EvalParams,
		const FGameplayEffectSpec& Spec
	) const;
	
	void RouteGuardDamage(
		float FinalDamage,
		UAbilitySystemComponent* TargetASC,
		const FAggregatorEvaluateParameters& EvalParams,
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput
	) const;
};
