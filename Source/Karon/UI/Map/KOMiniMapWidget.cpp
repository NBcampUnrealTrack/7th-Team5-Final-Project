#include "UI/Map/KOMiniMapWidget.h"

#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "KOMapUIComponent.h"

UKOMiniMapWidget::UKOMiniMapWidget()
{
    InputMode = EKOUIInputMode::Game;
    bIsBackHandler = false;
}

void UKOMiniMapWidget::SetMapUIComponent(UKOMapUIComponent* InMapUIComponent)
{
    MapUIComponent = InMapUIComponent;

    CacheMapData();
    ApplyMaterialParams();
}

void UKOMiniMapWidget::NativePreConstruct()
{
    Super::NativePreConstruct();

    if (!MiniMapMaterial || !ImageMiniMapTexture)
    {
        return;
    }

    MiniMapMID = UMaterialInstanceDynamic::Create(MiniMapMaterial, this);
    ImageMiniMapTexture->SetBrushFromMaterial(MiniMapMID);
}

void UKOMiniMapWidget::NativeConstruct()
{
    Super::NativeConstruct();

    CacheMapData();
    ApplyMaterialParams();
}

void UKOMiniMapWidget::CacheMapData()
{
    if (!MapUIComponent)
    {
        return;
    }

    MapTexture = MapUIComponent->GetMapTexture();
    LevelDimension = MapUIComponent->GetLevelDimension();
    MiniMapZoom = MapUIComponent->GetMiniMapZoom();
}

void UKOMiniMapWidget::ApplyMaterialParams()
{
    if (!MiniMapMID)
    {
        return;
    }

    MiniMapMID->SetTextureParameterValue(TEXT("Map Texture"), MapTexture);
    MiniMapMID->SetScalarParameterValue(TEXT("Zoom"), MiniMapZoom);
    MiniMapMID->SetScalarParameterValue(TEXT("Dimension"), LevelDimension);
}

void UKOMiniMapWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    UpdateMiniMap();
}

void UKOMiniMapWidget::UpdateMiniMap()
{
    if (!MiniMapMID || !ImagePlayerArrow)
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

    MiniMapMID->SetScalarParameterValue(TEXT("X"), Location.X);
    MiniMapMID->SetScalarParameterValue(TEXT("Y"), Location.Y);

    ImagePlayerArrow->SetRenderTransformAngle(Rotation.Yaw);
}