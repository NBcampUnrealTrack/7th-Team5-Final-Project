#pragma once

#include "CoreMinimal.h"
#include "KOAttributeSetBase.h"
#include "KOMovementSet.generated.h"

UCLASS()
class KARON_API UKOMovementSet : public UKOAttributeSetBase
{
	GENERATED_BODY()
	
public:
	UKOMovementSet();
	
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	virtual void PostAttributeBaseChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) const override;
	
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;

public:
	ATTRIBUTE_ACCESSORS_BASIC(UKOMovementSet, MaxWalkSpeed);
	ATTRIBUTE_ACCESSORS_BASIC(UKOMovementSet, MaxAcceleration); 
	
	ATTRIBUTE_ACCESSORS_BASIC(UKOMovementSet, BrakingDecelerationWalking);
	ATTRIBUTE_ACCESSORS_BASIC(UKOMovementSet, GroundFriction); 
	ATTRIBUTE_ACCESSORS_BASIC(UKOMovementSet, MaxWalkSpeedCrouched)
	
	ATTRIBUTE_ACCESSORS_BASIC(UKOMovementSet, JumpStrength)
protected:
	// ─── Attributes ────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData MaxWalkSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData MaxAcceleration;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData BrakingDecelerationWalking;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData GroundFriction;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData MaxWalkSpeedCrouched;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	FGameplayAttributeData JumpStrength;
	
public:
	// ─── Delegates ────────────────────────────────────────────────────
	FOnAttributeChanged OnMaxWalkSpeedBaseChanged;
	FOnAttributeChanged OnMaxAccelerationChanged;
	
	FOnAttributeChanged OnBrakingDecelerationWalkingChanged;
	FOnAttributeChanged OnGroundFrictionChanged;

	FOnAttributeChanged OnMaxWalkSpeedCrouchedChanged;
	FOnAttributeChanged OnJumpStrengthChanged;
};
