#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOMiniMapWidget.generated.h"

class UImage;
class USizeBox;
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UTexture2D;
class UKOMapUIComponent;

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOMiniMapWidget : public UKOActivatableWidget
{
	GENERATED_BODY()

public:
	UKOMiniMapWidget();

	void SetMapUIComponent(UKOMapUIComponent* InMapUIComponent);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImageMiniMapTexture;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImagePlayerArrow;

	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<USizeBox> SizeBoxMiniMap;

	UPROPERTY(EditDefaultsOnly, Category="KO|Map")
	TObjectPtr<UMaterialInterface> MiniMapMaterial;

private:
	void CacheMapData();
	void ApplyMaterialParams();
	void UpdateMiniMap();

	UPROPERTY()
	TObjectPtr<UKOMapUIComponent> MapUIComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> MiniMapMID;

	UPROPERTY()
	TObjectPtr<UTexture2D> MapTexture;

	float LevelDimension = 160000.f;
	float MiniMapZoom = 0.2f;
};