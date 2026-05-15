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
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
public:
	ATTRIBUTE_ACCESSORS_BASIC(UKOMovementSet, MoveSpeed);
	ATTRIBUTE_ACCESSORS_BASIC(UKOMovementSet, MaxMoveSpeed); 
	ATTRIBUTE_ACCESSORS_BASIC(UKOMovementSet, JumpStrength)
	
protected:
	// ─── Attributes ────────────────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing=OnRep_MoveSpeed, meta = (Units = "cm/s"))
	FGameplayAttributeData MoveSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing=OnRep_MaxMoveSpeed, meta = (Units = "cm/s"))
	FGameplayAttributeData MaxMoveSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category = "Movement", ReplicatedUsing=OnRep_JumpStrength)
	FGameplayAttributeData JumpStrength;
	
public:
	// ─── Delegates ────────────────────────────────────────────────────
	FOnAttributeChanged OnMoveSpeedBaseChanged;
	FOnAttributeChanged OnMoveSpeedChanged;
	
	FOnAttributeChanged OnMaxMoveSpeedBaseChanged;
	FOnAttributeChanged OnMaxMoveSpeedChanged;

	FOnAttributeChanged OnJumpStrengthBaseChanged;
	FOnAttributeChanged OnJumpStrengthChanged;
	
private:
	// ─── OnReps ────────────────────────────────────────────────────
	UFUNCTION() 
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);
	
	UFUNCTION()
	void OnRep_MaxMoveSpeed(const FGameplayAttributeData& OldMaxMoveSpeed);
	
	UFUNCTION()
	void OnRep_JumpStrength(const FGameplayAttributeData& OldJumpStrength);
};
