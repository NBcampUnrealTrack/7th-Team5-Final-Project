#include "KOTitleGameMode.h"
#include "KOTitleController.h"

AKOTitleGameMode::AKOTitleGameMode()
{
	DefaultPawnClass = nullptr;
	
	PlayerControllerClass = AKOTitleController::StaticClass();
}
