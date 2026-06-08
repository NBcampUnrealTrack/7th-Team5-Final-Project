#include "KOMapUIComponent.h"

#include "CommonActivatableWidget.h"
#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"
#include "UI/KOUISubsystem.h"
#include "UI/Map/KOMainMapWidget.h"

UKOMapUIComponent::UKOMapUIComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UKOMapUIComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UKOMapUIComponent::ToggleMainMap()
{
	if (MainMapWidget && MainMapWidget->IsActivated())
	{
		CloseMainMap();
		return;
	}

	OpenMainMap();
}

void UKOMapUIComponent::OpenMainMap()
{
	UCommonActivatableWidget* Widget =
		UKOUISubsystem::OpenWidget(this, KOGameplayTags::UI_Widget_Map_MainMap);

	MainMapWidget = Cast<UKOMainMapWidget>(Widget);

	if (MainMapWidget)
	{
		MainMapWidget->SetMapUIComponent(this);
	}
}

void UKOMapUIComponent::CloseMainMap()
{
	UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_Map_MainMap);
	MainMapWidget = nullptr;
}
