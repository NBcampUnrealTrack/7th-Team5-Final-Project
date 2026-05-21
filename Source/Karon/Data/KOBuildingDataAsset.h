#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KOBuildingDataAsset.generated.h"

class AActor;
class AKOBaseBuilding;

UCLASS(BlueprintType)
class KARON_API UKOBuildingDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	FName BuildingID = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Building")
	TSubclassOf<AKOBaseBuilding> BuildingClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	FIntPoint GridSize = FIntPoint(1, 1);

	// 바닥보다 살짝 위/아래로 보정할 값
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	float PlacementZOffset = 0.0f;
};