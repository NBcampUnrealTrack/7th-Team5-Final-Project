// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "KOTeleportSubsystem.generated.h"

class AKO_ABonfire;

/**
 * 
 */

USTRUCT()
struct FBonfireData
{
	GENERATED_BODY()
	
	UPROPERTY()
	FName BonfireID;
	
	UPROPERTY()
	FText DisplayName;
	
	UPROPERTY()
	int32 DisplayOrder;
	
	TWeakObjectPtr<AKO_ABonfire> Actor;
};

USTRUCT()
struct FBonfireUIData
{
	GENERATED_BODY()
	
	UPROPERTY()
	FName BonfireID;
	
	UPROPERTY()
	FText DisplayName;
	
	UPROPERTY()
	int32 DisplayOrder;
};

UCLASS()
class KARON_API UKOTeleportSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	
	void RegisterBonfire(AKO_ABonfire* InBonfire);
	
	bool ActivateBonfire(const FName& BonfireID);
	bool IsActivated(const FName& BonfireID) const;
	
	TArray<FBonfireUIData> GetActivatedBonfires() const;
	
	
	const TSet<FName>& GetActivatedBonfireSet() const;
	
	void SetActivatedBonfireSet(const TSet<FName>& NewSet);
	
private:
	UPROPERTY()
	TMap<FName, FBonfireData> BonfireMap;
	
	UPROPERTY()
	TSet<FName> ActivatedBonfires;
};
