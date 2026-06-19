#pragma once

#include "CoreMinimal.h"
#include "Animation/KOAnimationTypes.h"
#include "Character/KOCharacterBase.h"
#include "KOHeroCharacter.generated.h"

class UMotionWarpingComponent;
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
	
	
	virtual void OnCharacterDead(AActor* DeathInstigator) override;

public:
	UFUNCTION(BlueprintCallable)
	UCharacterTrajectoryComponent* GetTrajectoryComponent() const { return Trajectory; }
	
	UFUNCTION(BlueprintCallable)
	UMotionWarpingComponent* GetMotionWarpingComponent() const { return MotionWarpingComponent; }
	
	UFUNCTION(BlueprintCallable)
	UKOStaminaSet* GetStaminaSet() const { return StaminaSet; }
	
	UFUNCTION(BlueprintCallable)
	UKOCombatSet* GetCombatSet() const { return CombatSet; }
	
	UFUNCTION(BlueprintCallable)
	UKOHealthSet* GetHealthSet() const { return HealthSet; }
	
	UFUNCTION(BlueprintCallable)
	UKOMovementSet* GetMovementSet() const { return MovementSet; }
	
	UFUNCTION(BlueprintCallable)
	bool IsLockOn() const;
	
public:
	UFUNCTION(BlueprintCallable, Category = "MotionWarping")
	void SetMotionWarpTarget(const FName& WarpTargetName);
	
	UFUNCTION(BlueprintCallable, Category = "MotionWarping")
	void SetMotionWarpTargetWithLocation(const FName& WarpTargetName, const FVector& Location);
	
	UFUNCTION(BlueprintCallable, Category = "MotionWarping")
	void ClearMotionWarpTarget(const FName& WarpTargetName);
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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionWarping")
	TObjectPtr<UMotionWarpingComponent> MotionWarpingComponent;

protected:
	UPROPERTY()
	TObjectPtr<UKOStaminaSet> StaminaSet;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Reference")
	TObjectPtr<UAnimInstance> MainAnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion")
	EGait CurrentGait = EGait::Run; 
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Locomotion")
	EGait PreviousGait;
};
