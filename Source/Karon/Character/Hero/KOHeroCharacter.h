#pragma once

#include "CoreMinimal.h"
#include "Character/KOCharacterBase.h"
#include "KOHeroCharacter.generated.h"

class UKOLockOnComponent;
class USpringArmComponent;
class UCameraComponent;
class UKOPreCMCTickComponent;
class UCharacterTrajectoryComponent;
class UKOStaminaSet; 
class UKOCombatSet;

UCLASS()
class KARON_API AKOHeroCharacter : public AKOCharacterBase
{
	GENERATED_BODY()

public:
	AKOHeroCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay() override;
	
	virtual void PossessedBy(AController* NewController) override;

public:
	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	
public:
	UFUNCTION(BlueprintCallable)
	UKOLockOnComponent* GetLockOnComponent() const { return LockOnComponent; }
	
	UFUNCTION(BlueprintCallable)
	UCharacterTrajectoryComponent* GetTrajectoryComponent() const { return Trajectory; }
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USpringArmComponent> SprintArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UCameraComponent> Camera;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UKOPreCMCTickComponent> PreCMCTick;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UCharacterTrajectoryComponent> Trajectory;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UKOLockOnComponent> LockOnComponent;
	
protected:
	UPROPERTY()
	TObjectPtr<UKOStaminaSet> StaminaSet;
	
	UPROPERTY()
	TObjectPtr<UKOCombatSet> CombatSet;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Reference")
	TObjectPtr<UAnimInstance> MainAnimInstance;
};
