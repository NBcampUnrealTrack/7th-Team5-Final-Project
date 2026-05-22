#include "KOTitleController.h"

#include "CommonUserWidget.h"
#include "UI/KOUISubsystem.h"
#include "UI/KOActivatableWidget.h"

#include "GameplayTagContainer.h"
#include "Widgets/CommonActivatablewidgetContainer.h"

void AKOTitleController::BeginPlay()
{
	Super::BeginPlay();
	
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	
	CreateRootLayout();
	PushInitialWidgets();
}

void AKOTitleController::CreateRootLayout()
{
	if (!RootLayoutClass) return;
	
	RootLayOutInstance = CreateWidget<UKOActivatableWidget>(this, RootLayoutClass);
	
	if (RootLayOutInstance)
	{
		RootLayOutInstance->AddToRoot();
	}
}

void AKOTitleController::PushInitialWidgets() const
{
	auto* KOUISubsystem = GetLocalPlayer()->GetSubsystem<UKOUISubsystem>();
	FGameplayTag TitleTag = FGameplayTag::RequestGameplayTag(TEXT("UI.Layer.Menu"));
	KOUISubsystem->PushLayer(TitleTag, TitleWidgetClass);
}
