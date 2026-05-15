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
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	ATTRIBUTE_ACCESSORS_BASIC(UKOHealthSet, Health); 
	ATTRIBUTE_ACCESSORS_BASIC(UKOHealthSet, MaxHealth); 
	
	ATTRIBUTE_ACCESSORS_BASIC(UKOHealthSet, Damage)
	ATTRIBUTE_ACCESSORS_BASIC(UKOHealthSet, Healing)
	
protected:
	// ─── Attributes ────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly ,ReplicatedUsing = OnRep_Health,  Category = "Health")
	FGameplayAttributeData Health; 
	
	UPROPERTY(BlueprintReadOnly ,ReplicatedUsing= OnRep_MaxHealth, Category = "Health")
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

private:
	// ─── OnReps ────────────────────────────────────────────────────
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth); 
	
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth); 
};
