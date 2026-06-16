#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "KOComboActionData.generated.h"

USTRUCT(BlueprintType)
struct FKOComboActionData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combo|Action")
	TObjectPtr<UAnimMontage> LightAttackMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combo|Action")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 MaxLightComboCount;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	int32 MaxHeavyComboCount;
};
