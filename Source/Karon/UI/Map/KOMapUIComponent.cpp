#include "KOMapUIComponent.h"

#include "CommonActivatableWidget.h"
#include "AbilitySystem/Tag/UI/KOGameplayTags_UI.h"
#include "FOW/KOFogManagerSubsystem.h"
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
 
UKOFogManagerSubsystem* UKOMapUIComponent::ResolveFogManager() const
{
	if (!CachedFogManager)
	{
		CachedFogManager = UKOFogManagerSubsystem::Get(this);
	}
	return CachedFogManager;
}
 
UTextureRenderTarget2D* UKOMapUIComponent::GetExploredFogRT() const
{
	UKOFogManagerSubsystem* FogManager = ResolveFogManager();
	return FogManager ? FogManager->GetExploredFogRT() : nullptr;
}
 
UTextureRenderTarget2D* UKOMapUIComponent::GetVisibleFogRT() const
{
	UKOFogManagerSubsystem* FogManager = ResolveFogManager();
	return FogManager ? FogManager->GetVisibleFogRT() : nullptr;
}
 
FVector2D UKOMapUIComponent::GetFogMapOrigin() const
{
	// FogManager 오브젝트 자체는 LocalPlayer 서브시스템이라 레벨에 배치된 액터가 없어도 항상
	// 존재한다. RegisterConfig()가 실제로 호출되기 전(IsConfigured()==false)에는 미설정
	// 기본값(원점 0,0)을 갖고 있을 뿐이므로, 그걸 "레벨의 실제 원점"으로 오인하면 안 된다.
	UKOFogManagerSubsystem* FogManager = ResolveFogManager();
	return (FogManager && FogManager->IsConfigured()) ? FogManager->GetMapOrigin() : FVector2D::ZeroVector;
}

FVector2D UKOMapUIComponent::GetFogMapSize() const
{
	// 위와 동일한 이유로, 레벨에 FogManager가 배치되어 있지 않으면(미설정 상태) 이 컴포넌트에
	// 직접 설정된 LevelDimensionX/Y로 폴백한다. FogManager->GetMapSize()의 미설정 기본값
	// (10000x10000)을 그대로 믿으면 실제 레벨 크기와 어긋나 맵/아이콘 위치가 크게 틀어진다.
	UKOFogManagerSubsystem* FogManager = ResolveFogManager();
	return (FogManager && FogManager->IsConfigured()) ? FogManager->GetMapSize() : FVector2D(LevelDimensionX, LevelDimensionY);
}
