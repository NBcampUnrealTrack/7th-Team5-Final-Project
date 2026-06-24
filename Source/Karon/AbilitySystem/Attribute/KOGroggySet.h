#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attribute/KOAttributeSetBase.h"
#include "KOGroggySet.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnGroggyTriggered);

UCLASS()
class KARON_API UKOGroggySet : public UKOAttributeSetBase
{
	GENERATED_BODY()
 
public:
	UKOGroggySet();
 
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
 
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
 
	virtual void PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const override;
 
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
 
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
 
private:
	void HandleGroggyDamage(const FGameplayEffectModCallbackData& Data);
 
public:
	ATTRIBUTE_ACCESSORS_BASIC(UKOGroggySet, GroggyHealth)
	ATTRIBUTE_ACCESSORS_BASIC(UKOGroggySet, MaxGroggyHealth)
	ATTRIBUTE_ACCESSORS_BASIC(UKOGroggySet, GroggyDamage)
 
public:
	// ─── Attributes ───────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Groggy")
	FGameplayAttributeData GroggyHealth;
 
	UPROPERTY(BlueprintReadOnly, Category = "Groggy")
	FGameplayAttributeData MaxGroggyHealth;
	
	UPROPERTY(BlueprintReadOnly, Category = "Groggy")
	FGameplayAttributeData GroggyDamage;
 
public:
	// ─── Delegates ────────────────────────────────────────────────
	FOnAttributeChanged OnGroggyHealthChanged;
	FOnAttributeChanged OnGroggyHealthBaseChanged;
 
	FOnAttributeChanged OnMaxGroggyHealthChanged;
	FOnAttributeChanged OnMaxGroggyHealthBaseChanged;
 
	// 그로기 수치 0 도달 시
	FOnGroggyTriggered OnGroggyTriggered;
	
};
