// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Subsystem/KOTeleportSubsystem.h"
#include "KOBonfireEntryObject.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class KARON_API UKOBonfireEntryObject : public UObject
{
	GENERATED_BODY()
	
public:
	
	void Initialize(const FBonfireUIData& InData);
	
	UFUNCTION(BlueprintPure, Category = "Bonfire")
	const FName& GetBonfireID() const;
	
	UFUNCTION(BlueprintPure, Category = "Bonfire")
	const FText& GetDisplayName() const;
	
private:
	
	UPROPERTY()
	FBonfireUIData BonfireData;
};
