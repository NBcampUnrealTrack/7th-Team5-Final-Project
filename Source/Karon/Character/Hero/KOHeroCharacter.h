#pragma once

#include "CoreMinimal.h"
#include "Character/KOCharacterBase.h"
#include "KOHeroCharacter.generated.h"

class UKOStaminaSet; 
class UKOCombatSet;
class UKOLockOnComponent;  
class UKOInputConfig;

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
	
	// 외부(Ability 등)에서 접근용
	UFUNCTION(BlueprintCallable)
	UKOLockOnComponent* GetLockOnComponent() const { return LockOnComponent; }
	
protected:
	UPROPERTY()
	TObjectPtr<UKOStaminaSet> StaminaSet;
	
	UPROPERTY()
	TObjectPtr<UKOCombatSet> CombatSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	TObjectPtr<UKOLockOnComponent> LockOnComponent;
	
	// InputConfig 에셋 참조 추가
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UKOInputConfig> InputConfig;

private:
	TArray<uint32> BindHandles;

	void Input_AbilityInputTagPressed(FGameplayTag InputTag);
	void Input_AbilityInputTagReleased(FGameplayTag InputTag);
};
