#pragma once

#include "CoreMinimal.h"
#include "KOAttributeSetBase.h"
#include "KOCombatSet.generated.h"


UCLASS()
class KARON_API UKOCombatSet : public UKOAttributeSetBase
{
	GENERATED_BODY()

public:
	UKOCombatSet();

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

public:
	ATTRIBUTE_ACCESSORS_BASIC(UKOCombatSet, AttackPower);
	ATTRIBUTE_ACCESSORS_BASIC(UKOCombatSet, Defense);
	ATTRIBUTE_ACCESSORS_BASIC(UKOCombatSet, AttackSpeed);

	//현석: DEFINE_ATTRIBUTE_CAPTUREDEF 매크로를 사용하기 위해 public 선언
public:
	// ─── Attributes ────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	FGameplayAttributeData AttackPower;

	UPROPERTY(BlueprintReadOnly, Category = "Defense")
	FGameplayAttributeData Defense;

	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	FGameplayAttributeData AttackSpeed;

public:
	// ─── Delegates ────────────────────────────────────────────────────
	FOnAttributeChanged OnAttackPowerBaseChanged;
	FOnAttributeChanged OnAttackPowerChanged;

	FOnAttributeChanged OnDefenseBaseChanged;
	FOnAttributeChanged OnDefenseChanged;

	FOnAttributeChanged OnAttackSpeedChanged;
	FOnAttributeChanged OnAttackSpeedBaseChanged;
};
