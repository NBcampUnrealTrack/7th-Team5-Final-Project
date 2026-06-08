#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOMainMapWidget.generated.h"

class UImage;
class UKOMapUIComponent;

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOMainMapWidget : public UKOActivatableWidget
{
	GENERATED_BODY()

public:
	UKOMainMapWidget();

	void SetMapUIComponent(UKOMapUIComponent* InMapUIComponent);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ImagePlayerIcon;

private:
	void UpdatePlayerIcon();

	UPROPERTY()
	TObjectPtr<UKOMapUIComponent> MapUIComponent;

	float LevelDimension = 20000.f;
	float CanvasDimension = 1000.f;
};