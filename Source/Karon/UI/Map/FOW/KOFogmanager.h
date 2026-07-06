#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOFogManager.generated.h"

class UKOVisionComponent;
class UTextureRenderTarget2D;
class UMaterialInterface;
class UMaterialInstanceDynamic;
 
UCLASS()
class KARON_API AKOFogManager : public AActor
{
	GENERATED_BODY()
 
public:
	AKOFogManager();
 
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
 
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog")
	UTextureRenderTarget2D* CurrentFogRT;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog")
	UTextureRenderTarget2D* ExploredFogRT;
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog")
	UMaterialInterface* DrawMaterial;
 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fog")
	UMaterialInterface* CombineMaterial;
 
	UPROPERTY(EditAnywhere, Category="Fog")
	FVector2D MapSize = FVector2D(10000.f, 10000.f);
 
	UPROPERTY(EditAnywhere, Category="Fog")
	FVector2D MapOrigin = FVector2D(0.f, 0.f);
 
	UPROPERTY(EditAnywhere, Category="Fog")
	float UpdateInterval = 0.1f;
 
public:
	void RegisterVision(UKOVisionComponent* Comp);
	void UnregisterVision(UKOVisionComponent* Comp);
 
	UFUNCTION(BlueprintPure, Category="Fog")
	bool IsLocationVisible(const FVector& WorldLocation) const;
 
	UFUNCTION(BlueprintPure, Category="Fog")
	bool IsLocationExplored(const FVector& WorldLocation) const;
 
	FVector2D WorldToUV(const FVector& WorldPos) const;
 
private:
	void UpdateFog();
	bool ReadPixelsFromRT(UTextureRenderTarget2D* RT, TArray<FColor>& OutPixels) const;
 
	UPROPERTY()
	UMaterialInstanceDynamic* DrawMID;
 
	UPROPERTY()
	UMaterialInstanceDynamic* CombineMID;
 
	UPROPERTY()
	TArray<TWeakObjectPtr<UKOVisionComponent>> RegisteredVisionComponents;
 
	float AccumulatedTime = 0.f;
	
	TArray<FColor> CachedVisiblePixels;
	TArray<FColor> CachedExploredPixels;
	int32 CachedSizeX = 0;
	int32 CachedSizeY = 0;
};
