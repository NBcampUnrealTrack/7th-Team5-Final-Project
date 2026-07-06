#include "KOMapUIComponent.h"

#include "CommonActivatableWidget.h"
#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"
#include "FOW/KOFogManager.h"
#include "Kismet/GameplayStatics.h"
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
	if (UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(this))
	{
		if (UISubsystem->FindActiveWidget(KOGameplayTags::UI_Widget_Map_MainMap))
		{
			CloseMainMap();
			return;
		}
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
 
AKOFogManager* UKOMapUIComponent::ResolveFogManager() const
{
	if (!CachedFogManager)
	{
		CachedFogManager = Cast<AKOFogManager>(
			UGameplayStatics::GetActorOfClass(GetWorld(), AKOFogManager::StaticClass()));
	}
	return CachedFogManager;
}
 
UTextureRenderTarget2D* UKOMapUIComponent::GetExploredFogRT() const
{
	AKOFogManager* FogManager = ResolveFogManager();
	return FogManager ? FogManager->ExploredFogRT : nullptr;
}
 
UTextureRenderTarget2D* UKOMapUIComponent::GetVisibleFogRT() const
{
	AKOFogManager* FogManager = ResolveFogManager();
	return FogManager ? FogManager->CurrentFogRT : nullptr;
}
 
FVector2D UKOMapUIComponent::GetFogMapOrigin() const
{
	AKOFogManager* FogManager = ResolveFogManager();
	return FogManager ? FogManager->MapOrigin : FVector2D::ZeroVector;
}
 
FVector2D UKOMapUIComponent::GetFogMapSize() const
{
	AKOFogManager* FogManager = ResolveFogManager();
	return FogManager ? FogManager->MapSize : FVector2D(LevelDimensionX, LevelDimensionY);
}
