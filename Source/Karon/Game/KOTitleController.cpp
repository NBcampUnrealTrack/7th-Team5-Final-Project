#include "KOTitleController.h"

void AKOTitleController::BeginPlay()
{
	Super::BeginPlay();
	
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
}
