
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "KOCharacterStatRow.generated.h"

USTRUCT(BlueprintType)
struct FKOMovementStatData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float MaxWalkSpeed = 400.f;

	UPROPERTY(EditDefaultsOnly)
	float MaxAcceleration = 800.f;

	UPROPERTY(EditDefaultsOnly)
	float GroundFriction = 5.f;

	UPROPERTY(EditDefaultsOnly)
	float BrakingDecelerationWalking = 2000.f;

	UPROPERTY(EditDefaultsOnly)
	float MaxWalkSpeedCrouched = 225.f;

	UPROPERTY(EditDefaultsOnly)
	float JumpStrength = 600.f;
};

USTRUCT(BlueprintType)
struct FKOCombatStatData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	float Attack = 10.f;

	UPROPERTY(EditDefaultsOnly)
	float AttackSpeed = 1.f;

	UPROPERTY(EditDefaultsOnly)
	float CritChance = 0.3f;

	UPROPERTY(EditDefaultsOnly)
	float CritMultiplier = 1.5f;

	UPROPERTY(EditDefaultsOnly)
	float Defense = 5.f;
};

USTRUCT(BlueprintType)
struct FKOCharacterStatRow : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, Category = "Health")
	float MaxHP = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Stamina")
	float MaxStamina = 100.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Movement")
	FKOMovementStatData Movement;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	FKOCombatStatData Combat;
};
