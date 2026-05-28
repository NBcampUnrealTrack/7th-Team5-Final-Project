// Copyright Karon Team 5. All Rights Reserved.
#include "KOInteractionComponent.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/KOInteractableInterface.h"

UKOInteractionComponent::UKOInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UKOInteractionComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	UpdateCurrentInteractable();
}

void UKOInteractionComponent::UpdateCurrentInteractable()
{
	AActor* NewTarget = nullptr;

	FHitResult Hit;
	if (TraceFromScreenCenter(Hit))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<UKOInteractableInterface>())
		{
			AActor* Interactor = nullptr;
			if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
			{
				Interactor = PC->GetPawn();
			}

			if (IKOInteractableInterface::Execute_CanInteract(HitActor, Interactor))
			{
				NewTarget = HitActor;
			}
		}
	}

	CurrentInteractable = NewTarget;
}

bool UKOInteractionComponent::TryInteract()
{
	AActor* Target = CurrentInteractable.Get();
	if (!Target || !Target->Implements<UKOInteractableInterface>())
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	AActor* Interactor = PC ? PC->GetPawn() : nullptr;

	if (!IKOInteractableInterface::Execute_CanInteract(Target, Interactor))
	{
		return false;
	}

	IKOInteractableInterface::Execute_OnInteract(Target, Interactor);
	return true;
}

bool UKOInteractionComponent::TraceFromScreenCenter(FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!World || !PC)
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * TraceDistance;

	FCollisionQueryParams Params;
	Params.bTraceComplex = false;
	if (APawn* Pawn = PC->GetPawn())
	{
		Params.AddIgnoredActor(Pawn);
	}

	if (TraceRadius > 0.f)
	{
		const FCollisionShape Shape = FCollisionShape::MakeSphere(TraceRadius);
		return World->SweepSingleByChannel(
			OutHit, TraceStart, TraceEnd, FQuat::Identity, TraceChannel, Shape, Params);
	}

	return World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, TraceChannel, Params);
}
