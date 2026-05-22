#include "KOTitleController.h"
#include "UI/KOUISubsystem.h"

#include "CommonActivatableWidget.h"

void AKOTitleController::BeginPlay()
{
	Super::BeginPlay();
	
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (LocalPlayer)
	{
		UKOUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UKOUISubsystem>();
		if (UISubsystem)
		{
			//UISubsystem->RegisterPrimaryLayout( ,);
		}
	}
	
}
