#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayEffect.h"
#include "KOComboActionData.generated.h"


UENUM(BlueprintType)
enum class EAttackInputType : uint8
{
	None,
	Light,
	Heavy
};

USTRUCT(BlueprintType)
struct FKOComboActionData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combo|Action")
	TObjectPtr<UAnimMontage> ComboMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combo|Action")
	TSubclassOf<UGameplayEffect> DamageEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combo|Branch")
	FName NextLightRow;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combo|Branch")
	FName NextHeavyRow;
	
};
