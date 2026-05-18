#pragma once

#include "CoreMinimal.h"
#include "Character/KOCharacterBase.h"
#include "KOHeroCharacter.generated.h"

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
	
protected:
	UPROPERTY()
	TObjectPtr<UKOStaminaSet> StaminaSet;
	
	UPROPERTY()
	TObjectPtr<UKOCombatSet> CombatSet;
};
