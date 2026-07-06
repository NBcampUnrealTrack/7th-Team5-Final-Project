#include "UI/Map/KOMainMapWidget.h"
 
#include "Components/Image.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "KOMapUIComponent.h"
 
UKOMainMapWidget::UKOMainMapWidget()
{
	InputMode = EKOUIInputMode::All;
	bIsBackHandler = true;
}
 
void UKOMainMapWidget::SetMapUIComponent(UKOMapUIComponent* InMapUIComponent)
{
	MapUIComponent = InMapUIComponent;
 
	if (MapUIComponent)
	{
		LevelDimensionX = MapUIComponent->GetLevelDimensionX();
		LevelDimensionY = MapUIComponent->GetLevelDimensionY();
		CanvasDimension = MapUIComponent->GetMainMapCanvasDimension();
		MapOrigin = MapUIComponent->GetFogMapOrigin();
	}
 
	ApplyFogParams();
}
 
void UKOMainMapWidget::NativePreConstruct()
{
	Super::NativePreConstruct();
 
	if (!FogOverlayMaterial || !ImageFogOverlay)
	{
		return;
	}
 
	FogOverlayMID = UMaterialInstanceDynamic::Create(FogOverlayMaterial, this);
	ImageFogOverlay->SetBrushFromMaterial(FogOverlayMID);

	bFogParamsBound = false;
}
 
void UKOMainMapWidget::ApplyFogParams()
{
	if (!FogOverlayMID || !MapUIComponent || bFogParamsBound)
	{
		return;
	}
 
	UTextureRenderTarget2D* ExploredRT = MapUIComponent->GetExploredFogRT();
	UTextureRenderTarget2D* VisibleRT  = MapUIComponent->GetVisibleFogRT();
 
	if (!ExploredRT || !VisibleRT)
	{
		return;
	}
 
	FogOverlayMID->SetTextureParameterValue(TEXT("ExploredMask"), ExploredRT);
	FogOverlayMID->SetTextureParameterValue(TEXT("VisibleMask"), VisibleRT);
 
	bFogParamsBound = true;
}
 
void UKOMainMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
 
	if (!bFogParamsBound)
	{
		ApplyFogParams();
	}
 
	UpdatePlayerIcon();
}
 
void UKOMainMapWidget::UpdatePlayerIcon()
{
	if (!ImagePlayerIcon || LevelDimensionX <= 0.f || LevelDimensionY <= 0.f)
	{
		return;
	}
 
	APlayerController* PC = GetOwningPlayer();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	if (!Pawn)
	{
		return;
	}
 
	const FVector Location = Pawn->GetActorLocation();
	const FRotator Rotation = Pawn->GetActorRotation();

	const float ScaleX = CanvasDimension / LevelDimensionX;
	const float ScaleY = CanvasDimension / LevelDimensionY;

	const float HalfCanvas = CanvasDimension * 0.5f;
	ImagePlayerIcon->SetRenderTranslation(
		FVector2D(
			(Location.X - MapOrigin.X) * ScaleX - HalfCanvas,
			(Location.Y - MapOrigin.Y) * ScaleY - HalfCanvas
		)
	);
 
	ImagePlayerIcon->SetRenderTransformAngle(Rotation.Yaw);
}
