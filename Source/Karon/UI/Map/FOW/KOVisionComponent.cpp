#include "UI/Map/FOW/KOVisionComponent.h"
#include "UI/Map/FOW/KOFogManager.h"
#include "Kismet/GameplayStatics.h"
 
UKOVisionComponent::UKOVisionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
 
void UKOVisionComponent::BeginPlay()
{
	Super::BeginPlay();
 
	if (AKOFogManager* FogManager = Cast<AKOFogManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AKOFogManager::StaticClass())))
	{
		CachedFogManager = FogManager;
		FogManager->RegisterVision(this);
	}
}
 
void UKOVisionComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
 
	if (AKOFogManager* FogManager = CachedFogManager.Get())
	{
		FogManager->UnregisterVision(this);
	}
}
 
FVector UKOVisionComponent::GetVisionLocation() const
{
	return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}
 
