#include "Component/Interaction/KOInteractionComponent.h"

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
	IKOInteractableInterface* Target = CurrentInteractable.Get();
	if (!Target)
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	AActor* Interactor = PC ? PC->GetPawn() : nullptr;

	if (!Target->CanInteract(Interactor))
	{
		return false;
	}

	Target->OnInteract(Interactor);
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
