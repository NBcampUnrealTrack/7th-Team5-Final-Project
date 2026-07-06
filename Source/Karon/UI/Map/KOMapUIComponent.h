#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KOMapUIComponent.generated.h"

class AKOFogManager;
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
	
	float GetLevelDimension() const { return LevelDimensionX; }
 
	float GetLevelDimensionX() const { return LevelDimensionX; }
	float GetLevelDimensionY() const { return LevelDimensionY; }
 
	float GetMiniMapZoom() const { return MiniMapZoom; }
	float GetMainMapCanvasDimension() const { return MainMapCanvasDimension; }
	
	UTextureRenderTarget2D* GetExploredFogRT() const;
	UTextureRenderTarget2D* GetVisibleFogRT() const;
	FVector2D GetFogMapOrigin() const;
	FVector2D GetFogMapSize() const;
	
protected:
	UPROPERTY(EditAnywhere, Category="KO|Map")
	TObjectPtr<UTexture2D> MapTexture;
	
	UPROPERTY(EditAnywhere, Category="KO|Map")
	float LevelDimensionX = 150310.f;
	
	UPROPERTY(EditAnywhere, Category="KO|Map")
	float LevelDimensionY = 144460.f;
 
	UPROPERTY(EditAnywhere, Category="KO|Map")
	float MiniMapZoom = 0.2f;
 
	UPROPERTY(EditAnywhere, Category="KO|Map")
	float MainMapCanvasDimension = 1000.f;
 
private:
	UPROPERTY()
	TObjectPtr<UKOMainMapWidget> MainMapWidget;

	UPROPERTY()
	mutable TObjectPtr<AKOFogManager> CachedFogManager;
 
	AKOFogManager* ResolveFogManager() const;
};
