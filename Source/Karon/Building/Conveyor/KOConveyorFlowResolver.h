#pragma once

#include "CoreMinimal.h"

class UWorld;

class FKOConveyorFlowResolver
{
public:
	static FIntPoint WorldDirToGridStep(const FVector& Dir);
	static FVector GridStepToWorldDir(const FIntPoint& Step);
	static FIntPoint NegateStep(const FIntPoint& Step);

	static int32 ClassifyNeighbor(
		const UWorld* World,
		const FIntPoint& MyCell,
		const FIntPoint& NeighborCell
	);

	static bool TryResolveStraightReverse(
		const UWorld* World,
		const FIntPoint& MyCell,
		const FIntPoint& Forward,
		bool& OutReverse
	);

	static bool TryResolveCornerFlip(
		const UWorld* World,
		const FIntPoint& MyCell,
		const FIntPoint& Forward,
		const FIntPoint& Side,
		bool& OutFlip
	);
};