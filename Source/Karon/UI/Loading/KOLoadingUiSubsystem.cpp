// Copyright Karon Team 5. All Rights Reserved.

#include "KOLoadingUiSubsystem.h"
#include "CommonActivatableWidget.h"
#include "Engine/GameInstance.h"
#include "Engine/Engine.h"

void UKOLoadingUiSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UKOLoadingUiSubsystem::Deinitialize()
{
	HideLoadingScreen();
	Super::Deinitialize();
}

void UKOLoadingUiSubsystem::ShowLoadingScreen(TSubclassOf<UCommonActivatableWidget> LoadingWidgetClass)
{
	if (CurrentLoadingWidget || LoadingWidgetClass == nullptr)
	{
		return;
	}
	
	UWorld* World = GetGameInstance()->GetWorld();
	if (World == nullptr)
	{
		return;
	}
	
	CurrentLoadingWidget = CreateWidget<UCommonActivatableWidget>(World, LoadingWidgetClass);
	if (CurrentLoadingWidget)
	{
		CurrentLoadingWidget->AddToViewport(9999);
		CurrentLoadingWidget->ActivateWidget();
	}
}

void UKOLoadingUiSubsystem::HideLoadingScreen()
{
	if (CurrentLoadingWidget == nullptr)
	{
		return;
	}
	
	CurrentLoadingWidget->DeactivateWidget();
	CurrentLoadingWidget->RemoveFromParent();
	CurrentLoadingWidget = nullptr;
}
