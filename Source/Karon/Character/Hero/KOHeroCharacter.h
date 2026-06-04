#pragma once

#include "CoreMinimal.h"
#include "Animation/KOAnimationTypes.h"
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

public:
	UFUNCTION(BlueprintCallable)
	UCharacterTrajectoryComponent* GetTrajectoryComponent() const { return Trajectory; }
	
	
public:
	UFUNCTION(BlueprintCallable, Category= "Anim | Gait")
	EGait GetGait() const { return CurrentGait; }
	
	UFUNCTION(BlueprintCallable, Category= "Anim | Gait")
	void UpdateGait(EGait DesiredGait);

	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USpringArmComponent> SpringArm;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UCameraComponent> Camera;
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UKOPreCMCTickComponent> PreCMCTick;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UCharacterTrajectoryComponent> Trajectory;

protected:
	UPROPERTY()
	TObjectPtr<UKOStaminaSet> StaminaSet;
	
	UPROPERTY()
	TObjectPtr<UKOCombatSet> CombatSet;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Reference")
	TObjectPtr<UAnimInstance> MainAnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion")
	EGait CurrentGait = EGait::Run; 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion")
	EGait PreviousGait;

};
