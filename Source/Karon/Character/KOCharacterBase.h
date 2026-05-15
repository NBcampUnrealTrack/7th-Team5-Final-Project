#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/Character.h"

#include "KOCharacterBase.generated.h"

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
	
	virtual void BindMovementSet();
	
	UFUNCTION()
	virtual void OnWalkSpeedChanged(float NewWalkSpeed, float OldWalkSpeed);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Abilities")
	TObjectPtr<UKOAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atttribute | Health")
	TObjectPtr<UKOHealthSet> HealthSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atttribute | Movement")
	TObjectPtr<UKOMovementSet> MovementSet;
};
