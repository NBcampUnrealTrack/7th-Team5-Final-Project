#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KOMapUIComponent.generated.h"

class UKOMapSystemWidget;
class UTexture2D;
class UKOMainMapWidget;

UCLASS(ClassGroup="KO|Map", meta=(BlueprintSpawnableComponent))
class KARON_API UKOMapUIComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKOMapUIComponent();

	virtual void BeginPlay() override;

	void ToggleMainMap();
	void OpenMainMap();
	void CloseMainMap();

	UTexture2D* GetMapTexture() const { return MapTexture; }
	float GetLevelDimension() const { return LevelDimension; }
	float GetMiniMapZoom() const { return MiniMapZoom; }
	float GetMainMapCanvasDimension() const { return MainMapCanvasDimension; }

protected:
	UPROPERTY(EditAnywhere, Category="KO|Map")
	TObjectPtr<UTexture2D> MapTexture;

	UPROPERTY(EditAnywhere, Category="KO|Map")
	float LevelDimension = 20000.f;

	UPROPERTY(EditAnywhere, Category="KO|Map")
	float MiniMapZoom = 0.2f;

	UPROPERTY(EditAnywhere, Category="KO|Map")
	float MainMapCanvasDimension = 1000.f;

private:
	UPROPERTY()
	TObjectPtr<UKOMainMapWidget> MainMapWidget;
};