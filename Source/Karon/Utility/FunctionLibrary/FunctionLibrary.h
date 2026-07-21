#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "FunctionLibrary.generated.h"

UCLASS()
class KARON_API UFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static void FindActorsWithGameplayTagInRange(
		const UWorld* World,
		const FVector& ScanOrigin,
		float Radius,
		const FGameplayTag& TargetTag,
		const TArray<AActor*>& ActorsToIgnore,
		TArray<TWeakObjectPtr<AActor>>& OutDetectedActors
	);
	
	UFUNCTION(BlueprintCallable)
	static void SetUITextBlock(UObject* WorldContextObject,FGameplayTag Tag, FText Text);
	
	UFUNCTION(BlueprintCallable)
	static void DropItem(UObject* WorldContextObject, FName ItemName);
	
	UFUNCTION(BlueprintCallable)
	static bool HasMatchingTags(ACharacter* Character, TArray<FGameplayTag> Tags);
};