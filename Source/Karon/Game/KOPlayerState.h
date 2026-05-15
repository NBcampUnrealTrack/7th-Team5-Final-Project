#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "KOPlayerState.generated.h"

class UKOMovementSet;
class UKOCombatSet;
class UKOStaminaSet;
class UKOHealthSet;
class UKOAbilitySystemComponent;

UCLASS()
class KARON_API AKOPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	AKOPlayerState();
	
	// ── IAbilitySystemInterface ──
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	
	// ── Getter ──
	UKOHealthSet*    GetHealthSet()    const { return HealthSet; }
	UKOStaminaSet*   GetStaminaSet()   const { return StaminaSet; }
	UKOCombatSet*    GetCombatSet()    const { return CombatSet; }
	UKOMovementSet*  GetMovementSet()  const { return MovementSet; }
	
protected:
	virtual void BeginPlay() override;
	
	void InitializeAbilitySystem();

	
protected:
	// ─── Ability System ────────────────────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System")
	TObjectPtr<UKOAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System | Health")
	TObjectPtr<UKOHealthSet> HealthSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System | Stamina")
	TObjectPtr<UKOStaminaSet> StaminaSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System | Combat")
	TObjectPtr<UKOCombatSet> CombatSet;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability System | Movement")
	TObjectPtr<UKOMovementSet> MovementSet;
};
