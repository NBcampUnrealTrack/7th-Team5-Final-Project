#include "KOTitleGameMode.h"
#include "KOTitleController.h"

AKOTitleGameMode::AKOTitleGameMode()
{
	DefaultPawnClass = nullptr;
	
	HUDClass = nullptr;
	
	PlayerControllerClass = AKOTitleController::StaticClass();
}
