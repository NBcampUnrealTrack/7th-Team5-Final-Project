#pragma once

#include "CoreMinimal.h"
#include "KOAttributeSetBase.h"
#include "AbilitySystemComponent.h"
#include "KOGuardSet.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnGuardBreakTriggered);

UCLASS()
class KARON_API UKOGuardSet : public UKOAttributeSetBase
{
	GENERATED_BODY()
	
public:
	UKOGuardSet();

	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	virtual void PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const override;
	
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
	void HandleGuardDamage(const FGameplayEffectModCallbackData& Data);
public:
	ATTRIBUTE_ACCESSORS_BASIC(UKOGuardSet, GuardHealth);
	
	ATTRIBUTE_ACCESSORS_BASIC(UKOGuardSet, MaxGuardHealth);
	
	ATTRIBUTE_ACCESSORS_BASIC(UKOGuardSet, GuardDamage);
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Guard")
	FGameplayAttributeData GuardHealth;
	
	UPROPERTY(BlueprintReadOnly, Category = "Guard")
	FGameplayAttributeData MaxGuardHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Guard")
	FGameplayAttributeData GuardDamage;
	
public:
	FOnAttributeChanged OnGuardHealthBaseChanged;
	FOnAttributeChanged OnGuardHealthChanged;
	
	FOnAttributeChanged OnMaxGuardHealthChanged;
	FOnAttributeChanged OnMaxGuardHealthBaseChanged;
	
	FOnGuardBreakTriggered OnGuardBreakTriggered;
};
