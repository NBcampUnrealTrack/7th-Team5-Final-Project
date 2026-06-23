#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"

#include "KOCharacterBase.generated.h"

class UKOEquipmentComponent;
struct FInstancedStruct;
class FGameplayMessageCallback;
class UKOHealthSet;
class UKOMovementSet;
class UKOCombatSet;
class UKOAbilitySystemComponent; 

UCLASS()
class KARON_API AKOCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AKOCharacterBase(const FObjectInitializer& ObjectInitializer);
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	UKOEquipmentComponent* GetEquipmentComponent() const { return EquipmentComponent; }

	UKOMovementSet* GetMovementSet() const { return MovementSet; }
	
	UKOHealthSet* GetHealthSet() const { return HealthSet; }
	
	UKOCombatSet* GetCombatSet() const { return CombatSet; }
	
protected:
	virtual void BeginPlay() override;
	
	virtual void InitializeAttributes();

public:
	UFUNCTION()
	virtual void OnCharacterDead(AActor* DeathInstigator);
	
	UFUNCTION()
	void OnMaxWalkSpeedChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnMaxAccelerationChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnBrakingDecelerationChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnGroundFrictionChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnMaxWalkSpeedCrouchedChanged(float OldValue, float NewValue);
	
	UFUNCTION()
	void OnJumpStrengthChanged(float OldValue, float NewValue);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UKOAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atttribute | Combat")
	TObjectPtr<UKOCombatSet> CombatSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atttribute | Health")
	TObjectPtr<UKOHealthSet> HealthSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atttribute | Movement")
	TObjectPtr<UKOMovementSet> MovementSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UKOEquipmentComponent> EquipmentComponent;
 
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bIsDead = false; 
	
};
