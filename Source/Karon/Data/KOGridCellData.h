#pragma once

#include "CoreMinimal.h"
#include "KOGridCellData.generated.h"

class AActor;

USTRUCT(BlueprintType)
struct FKOGridCellData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool bIsBuildable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float TerrainZ = 0.0f;

	TWeakObjectPtr<AActor> OccupyingActor = nullptr;
};