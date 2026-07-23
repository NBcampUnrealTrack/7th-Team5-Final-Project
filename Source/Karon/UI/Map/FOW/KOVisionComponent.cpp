#include "UI/Map/FOW/KOVisionComponent.h"
#include "UI/Map/FOW/KOFogManagerSubsystem.h"

UKOVisionComponent::UKOVisionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UKOVisionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UKOFogManagerSubsystem* FogManager = UKOFogManagerSubsystem::Get(this))
	{
		CachedFogManager = FogManager;
		FogManager->RegisterVision(this);
	}
}

void UKOVisionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (UKOFogManagerSubsystem* FogManager = CachedFogManager.Get())
	{
		FogManager->UnregisterVision(this);
	}
}

FVector UKOVisionComponent::GetVisionLocation() const
{
	return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}