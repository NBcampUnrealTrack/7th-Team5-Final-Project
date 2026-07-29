// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Subsystem/KOTeleportSubsystem.h"
#include "KOBonfireEntryObject.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKOBonfireEntryObject : public UObject
{
	GENERATED_BODY()
	
public:
	
	void Initialize(const FBonfireUIData& InData);
	
	const FName& GetBonfireID() const;
	
	const FText& GetDisplayName() const;
	
private:
	
	UPROPERTY()
	FBonfireUIData BonfireData;
};
