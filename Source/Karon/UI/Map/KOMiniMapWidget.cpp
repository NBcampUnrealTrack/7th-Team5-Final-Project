#include "UI/Map/KOMiniMapWidget.h"

#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
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
    
    bFogParamsBound = false;
}
 
void UKOMiniMapWidget::NativeConstruct()
{
    Super::NativeConstruct();
 
    if (!MapUIComponent)
    {
        MapUIComponent = ResolveMapUIComponent();
    }
 
    CacheMapData();
    ApplyMaterialParams();
}
 
UKOMapUIComponent* UKOMiniMapWidget::ResolveMapUIComponent()
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return nullptr;
    }
    
    if (UKOMapUIComponent* FoundOnController = PC->FindComponentByClass<UKOMapUIComponent>())
    {
        return FoundOnController;
    }

    if (APawn* Pawn = PC->GetPawn())
    {
        if (UKOMapUIComponent* FoundOnPawn = Pawn->FindComponentByClass<UKOMapUIComponent>())
        {
            return FoundOnPawn;
        }
    }
 
    return nullptr;
}
 
void UKOMiniMapWidget::CacheMapData()
{
    if (!MapUIComponent)
    {
        return;
    }
 
    MapTexture = MapUIComponent->GetMapTexture();
    LevelDimensionX = MapUIComponent->GetLevelDimensionX();
    LevelDimensionY = MapUIComponent->GetLevelDimensionY();
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
    MiniMapMID->SetScalarParameterValue(TEXT("DimensionX"), LevelDimensionX);
    MiniMapMID->SetScalarParameterValue(TEXT("DimensionY"), LevelDimensionY);
    
    float CheckX = -9999.f;
    float CheckY = -9999.f;
    const bool bFoundX = MiniMapMID->GetScalarParameterValue(TEXT("DimensionX"), CheckX);
    const bool bFoundY = MiniMapMID->GetScalarParameterValue(TEXT("DimensionY"), CheckY);
 
    ApplyFogParams();
}
 
void UKOMiniMapWidget::ApplyFogParams()
{
    if (!MiniMapMID || !MapUIComponent || bFogParamsBound)
    {
        return;
    }
 
    UTextureRenderTarget2D* ExploredRT = MapUIComponent->GetExploredFogRT();
    UTextureRenderTarget2D* VisibleRT  = MapUIComponent->GetVisibleFogRT();
 
    if (!ExploredRT || !VisibleRT)
    {
        return;
    }
    
    MiniMapMID->SetTextureParameterValue(TEXT("ExploredMask"), ExploredRT);
    MiniMapMID->SetTextureParameterValue(TEXT("VisibleMask"), VisibleRT);
    
    UTexture* CheckExplored = nullptr;
    UTexture* CheckVisible  = nullptr;
    const bool bExploredFound = MiniMapMID->GetTextureParameterValue(TEXT("ExploredMask"), CheckExplored);
    const bool bVisibleFound  = MiniMapMID->GetTextureParameterValue(TEXT("VisibleMask"), CheckVisible);
 
    const FVector2D FogOrigin = MapUIComponent->GetFogMapOrigin();
    MiniMapMID->SetScalarParameterValue(TEXT("MapOriginX"), FogOrigin.X);
    MiniMapMID->SetScalarParameterValue(TEXT("MapOriginY"), FogOrigin.Y);

    bFogParamsBound = true;
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
 
    if (!MapUIComponent)
    {
        MapUIComponent = ResolveMapUIComponent();
        if (MapUIComponent)
        {
            CacheMapData();
            ApplyMaterialParams();
        }
    }
 
    if (!bFogParamsBound)
    {
        ApplyFogParams();
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
    
    if (MapUIComponent)
    {
        MiniMapMID->SetScalarParameterValue(TEXT("DimensionX"), MapUIComponent->GetLevelDimensionX());
        MiniMapMID->SetScalarParameterValue(TEXT("DimensionY"), MapUIComponent->GetLevelDimensionY());
    }
 
    ImagePlayerArrow->SetRenderTransformAngle(Rotation.Yaw);
}
