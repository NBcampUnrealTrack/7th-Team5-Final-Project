#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

namespace FunctionLibrary
{
	void FindActorsWithGameplayTagInRange(
		const UWorld* World,
		const FVector& ScanOrigin,
		float Radius,
		const FGameplayTag& TargetTag,
		const TArray<AActor*>& ActorsToIgnore,
		TArray<TWeakObjectPtr<AActor>>& OutDetectedActors
	);
}