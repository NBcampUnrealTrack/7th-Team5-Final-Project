#pragma once

#include "CoreMinimal.h"
#include "KOAttributeSetBase.h"
#include "KOHealthSet.generated.h"

UCLASS()
class KARON_API UKOHealthSet : public UKOAttributeSetBase
{
	GENERATED_BODY()
		
public:
	UKOHealthSet();
	
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	virtual void PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const override;
	
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
private:
	void HandleDamage(const FGameplayEffectModCallbackData& Data);
	
	void HandleDeath(const FGameplayEffectModCallbackData& Data);
	
	void HandleHealing(const FGameplayEffectModCallbackData& Data);
	
public:
	ATTRIBUTE_ACCESSORS_BASIC(UKOHealthSet, Health); 
	ATTRIBUTE_ACCESSORS_BASIC(UKOHealthSet, MaxHealth); 
	
	ATTRIBUTE_ACCESSORS_BASIC(UKOHealthSet, Damage)
	ATTRIBUTE_ACCESSORS_BASIC(UKOHealthSet, Healing)

public:
	// ─── Attributes ────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData Health; 
	
	UPROPERTY(BlueprintReadOnly, Category = "Health")
	FGameplayAttributeData MaxHealth; 
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	FGameplayAttributeData Damage;
	
	UPROPERTY(BlueprintReadOnly, Category = "Healing")
	FGameplayAttributeData Healing;
	
public:
	// ─── Delegates ────────────────────────────────────────────────────
	FOnAttributeChanged OnHealthBaseChanged;
	FOnAttributeChanged OnHealthChanged;
	
	FOnAttributeChanged OnMaxHealthBaseChanged;
	FOnAttributeChanged OnMaxHealthChanged;
	
};
