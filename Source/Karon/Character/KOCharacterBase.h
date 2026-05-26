#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Character.h"

#include "KOCharacterBase.generated.h"

struct FInstancedStruct;
class FGameplayMessageCallback;
class UKOHealthSet;
class UKOMovementSet;
class UKOAbilitySystemComponent; 

UCLASS()
class KARON_API AKOCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AKOCharacterBase(const FObjectInitializer& ObjectInitializer);
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
protected:
	virtual void BeginPlay() override;
	
	
protected:
	virtual void BindMovementSet();
	
	UFUNCTION()
	virtual void OnMoveSpeedChanged(float OldWalkSpeed, float NewWalkSpeed);
	
	UFUNCTION()
	virtual void OnJumpStrengthChanged(float OldJumpStrength, float NewJumpStrength);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UKOAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atttribute | Health")
	TObjectPtr<UKOHealthSet> HealthSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atttribute | Movement")
	TObjectPtr<UKOMovementSet> MovementSet;
	
 
};
