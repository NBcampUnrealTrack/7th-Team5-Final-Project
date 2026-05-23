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
		if (IKOInteractableInterface* Interactable = Cast<IKOInteractableInterface>(HitActor))
		{
			AActor* Interactor = nullptr;
			if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
			{
				Interactor = PC->GetPawn();
			}

			if (Interactable->CanInteract(Interactor))
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
	IKOInteractableInterface* Interactable = Cast<IKOInteractableInterface>(Target);
	if (!Interactable)
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	AActor* Interactor = PC ? PC->GetPawn() : nullptr;

	if (!Interactable->CanInteract(Interactor))
	{
		return false;
	}

	Interactable->OnInteract(Interactor);
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

	return World->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, TraceChannel, Params);
}
