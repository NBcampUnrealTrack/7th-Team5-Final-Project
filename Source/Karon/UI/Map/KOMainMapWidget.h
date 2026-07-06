#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOMainMapWidget.generated.h"

class UImage;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UKOMapUIComponent;
 
UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOMainMapWidget : public UKOActivatableWidget
{
	GENERATED_BODY()
 
public:
	UKOMainMapWidget();
 
	void SetMapUIComponent(UKOMapUIComponent* InMapUIComponent);
 
protected:
	virtual void NativePreConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
 
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImagePlayerIcon;
	
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UImage> ImageFogOverlay;
	
	UPROPERTY(EditDefaultsOnly, Category="KO|Map")
	TObjectPtr<UMaterialInterface> FogOverlayMaterial;
 
private:
	void UpdatePlayerIcon();
	void ApplyFogParams();
 
	UPROPERTY()
	TObjectPtr<UKOMapUIComponent> MapUIComponent;
 
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FogOverlayMID;
 
	float LevelDimensionX = 150310.f;
	float LevelDimensionY = 144460.f;
	float CanvasDimension = 1000.f;
 
	FVector2D MapOrigin = FVector2D::ZeroVector;
 
	bool bFogParamsBound = false;
};
 
