#include "UI/Map/FOW/KOFogManager.h"
#include "UI/Map/FOW/KOFogManagerSubsystem.h"

AKOFogManager::AKOFogManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AKOFogManager::BeginPlay()
{
	Super::BeginPlay();

	if (UKOFogManagerSubsystem* Subsystem = UKOFogManagerSubsystem::Get(this))
	{
		Subsystem->RegisterConfig(CurrentFogRT, ExploredFogRT, DrawMaterial, CombineMaterial, MapSize, MapOrigin, UpdateInterval);
	}
}