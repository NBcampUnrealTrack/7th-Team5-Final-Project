// Copyright Karon Team 5. All Rights Reserved.

#include "KOTeleportSubsystem.h"
#include "MapActor/KO_ABonfire.h"
#include "Character/KOCharacterBase.h"

UKOTeleportSubsystem* UKOTeleportSubsystem::Get(const UObject* WorldContext)
{
	if (!WorldContext)
	{
		return nullptr;
	}

	UWorld* World = WorldContext->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	ULocalPlayer* LocalPlayer = World->GetFirstLocalPlayerFromController();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	return LocalPlayer->GetSubsystem<UKOTeleportSubsystem>();
}

void UKOTeleportSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BonfireMap.Empty();
	ActivatedBonfires.Empty();
}

void UKOTeleportSubsystem::Deinitialize()
{
	BonfireMap.Empty();
	ActivatedBonfires.Empty();

	Super::Deinitialize();
}

void UKOTeleportSubsystem::RegisterBonfire(AKO_ABonfire* InBonfire)
{
	if (IsValid(InBonfire) == false)
	{
		return;
	}

	FBonfireData Data;

	Data.BonfireID = InBonfire->GetBonfireID();
	Data.DisplayName = InBonfire->GetDisplayName();
	Data.DisplayOrder = InBonfire->GetDisplayOrder();
	Data.Actor = InBonfire;

	BonfireMap.Add(Data.BonfireID, MoveTemp(Data));
}

bool UKOTeleportSubsystem::ActivateBonfire(const FName& BonfireID)
{
	if (ActivatedBonfires.Contains(BonfireID))
	{
		return false;
	}

	ActivatedBonfires.Add(BonfireID);
	return true;
}

bool UKOTeleportSubsystem::IsActivated(const FName& BonfireID) const
{
	return ActivatedBonfires.Contains(BonfireID);
}

TArray<FBonfireUIData> UKOTeleportSubsystem::GetActivatedBonfires() const
{
	TArray<FBonfireUIData> Result;

	Result.Reserve(ActivatedBonfires.Num());

	for (const FName& ID : ActivatedBonfires)
	{
		const FBonfireData* Data = BonfireMap.Find(ID);

		if (Data == nullptr)
		{
			continue;
		}

		FBonfireUIData UIData;

		UIData.BonfireID = Data->BonfireID;
		UIData.DisplayName = Data->DisplayName;
		UIData.DisplayOrder = Data->DisplayOrder;

		Result.Add(MoveTemp(UIData));
	}

	Result.Sort(
		[](const FBonfireUIData& A, const FBonfireUIData& B)
		{
			return A.DisplayOrder < B.DisplayOrder;
		});

	return Result;
}

const TSet<FName>& UKOTeleportSubsystem::GetActivatedBonfireSet() const
{
	return ActivatedBonfires;
}

void UKOTeleportSubsystem::SetActivatedBonfireSet(const TSet<FName>& NewSet)
{
	ActivatedBonfires = NewSet;

	// 외형 갱신
	for (const TPair<FName, FBonfireData>& Pair : BonfireMap)
	{
		if (AKO_ABonfire* Bonfire = Pair.Value.Actor.Get())
		{
			Bonfire->UpdateBonfireVisuals();
		}
	}
}

void UKOTeleportSubsystem::Teleport(const FName& TargetBonfireID)
{
	const FBonfireData* TargetDestination = BonfireMap.Find(TargetBonfireID);

	if (TargetDestination == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportSubsystem: Can't find Destination Point"));
		return;
	}

	const APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld());
	if (PC == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportSubsystem: Can't find LocalPlayer"));
		return;
	}
	ACharacter* Player = Cast<ACharacter>(PC->GetPawn());

	if (Player == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportSubsystem: Can't find Character"));
		return;
	}

	const FVector TargetLocation =
		TargetDestination->Actor->GetActorLocation()
		- TargetDestination->Actor->GetActorForwardVector() * FarDistanceFromActor;

	Player->SetActorLocation(TargetLocation);
}
