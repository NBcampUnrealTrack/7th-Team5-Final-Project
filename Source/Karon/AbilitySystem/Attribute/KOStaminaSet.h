#pragma once

#include "CoreMinimal.h"
#include "KOAttributeSetBase.h"
#include "KOStaminaSet.generated.h"


UCLASS()
class KARON_API UKOStaminaSet : public UKOAttributeSetBase
{
	GENERATED_BODY()
public:
	UKOStaminaSet();
	
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	virtual void PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const override;
	
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	ATTRIBUTE_ACCESSORS_BASIC(UKOStaminaSet, Stamina); 
	ATTRIBUTE_ACCESSORS_BASIC(UKOStaminaSet, MaxStamina); 

	ATTRIBUTE_ACCESSORS_BASIC(UKOStaminaSet, StaminaDrain); 
	ATTRIBUTE_ACCESSORS_BASIC(UKOStaminaSet, StaminaRegen); 
	
protected:
	// ─── Attributes ────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly ,ReplicatedUsing = OnRep_Stamina, Category = "Stamina")
	FGameplayAttributeData Stamina; 
	
	UPROPERTY(BlueprintReadOnly ,ReplicatedUsing = OnRep_MaxStamina, Category = "Stamina")
	FGameplayAttributeData MaxStamina; 
	
	UPROPERTY(BlueprintReadOnly, Category= "Stamina")
	FGameplayAttributeData StaminaDrain; // 소모량(메타)
	
	UPROPERTY(BlueprintReadOnly, Category= "Stamina")
	FGameplayAttributeData StaminaRegen; // 회복량(메타)  
	
public:
	// ─── Delegates ────────────────────────────────────────────────────
	FOnAttributeChanged OnStaminaBaseChanged;
	FOnAttributeChanged OnStaminaChanged;
	
	FOnAttributeChanged OnMaxStaminaBaseChanged;
	FOnAttributeChanged OnMaxStaminaChanged;

private:
	// ─── OnReps ────────────────────────────────────────────────────
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina); 
	
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina); 
};
