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

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

public:
	ATTRIBUTE_ACCESSORS_BASIC(UKOCombatSet, AttackPower);
	ATTRIBUTE_ACCESSORS_BASIC(UKOCombatSet, Defense);
	ATTRIBUTE_ACCESSORS_BASIC(UKOCombatSet, AttackSpeed);

protected:
	// ─── Attributes ────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "Attack")
	FGameplayAttributeData AttackPower;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Defense, Category = "Defense")
	FGameplayAttributeData Defense;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackSpeed, Category = "Attack")
	FGameplayAttributeData AttackSpeed;

public:
	// ─── Delegates ────────────────────────────────────────────────────
	FOnAttributeChanged OnAttackPowerBaseChanged;
	FOnAttributeChanged OnAttackPowerChanged;

	FOnAttributeChanged OnDefenseBaseChanged;
	FOnAttributeChanged OnDefenseChanged;

	FOnAttributeChanged OnAttackSpeedChanged;
	FOnAttributeChanged OnAttackSpeedBaseChanged;

private:
	// ─── OnReps ────────────────────────────────────────────────────
	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);

	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldDefense);

	UFUNCTION()
	void OnRep_AttackSpeed(const FGameplayAttributeData& OldAttackSpeed);
};
