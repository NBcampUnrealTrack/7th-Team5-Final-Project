#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Tickable.h"
#include "KOFogManagerSubsystem.generated.h"

class UKOVisionComponent;
class UTextureRenderTarget2D;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/**
 * UKOFogManagerSubsystem
 * LocalPlayer 수명 동안 존재하는 Fog of War 런타임(Tick/Vision 등록/가시성 조회) 담당 서브시스템.
 * 레벨별 설정(RenderTarget/Material/MapSize/MapOrigin)은 레벨에 배치되는 AKOFogManager가
 * BeginPlay에서 RegisterConfig()를 통해 주입한다.
 */
UCLASS()
class KARON_API UKOFogManagerSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	static UKOFogManagerSubsystem* Get(const UObject* WorldContextObject);

	virtual void Deinitialize() override;
	virtual UWorld* GetWorld() const override;

	// ~FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	// ~FTickableGameObject

	/** 레벨에 배치된 AKOFogManager(Config Actor)가 BeginPlay에서 호출해 레벨별 설정을 주입한다. */
	void RegisterConfig(
		UTextureRenderTarget2D* InCurrentFogRT,
		UTextureRenderTarget2D* InExploredFogRT,
		UMaterialInterface* InDrawMaterial,
		UMaterialInterface* InCombineMaterial,
		const FVector2D& InMapSize,
		const FVector2D& InMapOrigin,
		float InUpdateInterval);

	void RegisterVision(UKOVisionComponent* Comp);
	void UnregisterVision(UKOVisionComponent* Comp);

	UFUNCTION(BlueprintPure, Category="Fog")
	bool IsLocationVisible(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintPure, Category="Fog")
	bool IsLocationExplored(const FVector& WorldLocation) const;

	FVector2D WorldToUV(const FVector& WorldPos) const;

	UTextureRenderTarget2D* GetExploredFogRT() const { return ExploredFogRT; }
	UTextureRenderTarget2D* GetVisibleFogRT() const { return CurrentFogRT; }
	FVector2D GetMapOrigin() const { return MapOrigin; }
	FVector2D GetMapSize() const { return MapSize; }

	/**
	 * 레벨에 배치된 AKOFogManager가 BeginPlay에서 RegisterConfig()를 호출해 실제 값을 넣어주기
	 * 전에는 false. 이 값을 확인하지 않고 GetMapSize()/GetMapOrigin()을 그대로 믿으면, 레벨에
	 * FogManager가 하나도 없을 때 미설정 기본값(10000x10000, 원점 0,0)을 실제 값인 것처럼
	 * 잘못 사용하게 된다.
	 */
	bool IsConfigured() const { return bConfigured; }

private:
	void UpdateFog();
	bool ReadPixelsFromRT(UTextureRenderTarget2D* RT, TArray<FColor>& OutPixels) const;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> CurrentFogRT;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> ExploredFogRT;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> DrawMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInterface> CombineMaterial;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DrawMID;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> CombineMID;

	FVector2D MapSize = FVector2D(10000.f, 10000.f);
	FVector2D MapOrigin = FVector2D::ZeroVector;

	float UpdateInterval = 0.1f;
	float AccumulatedTime = 0.f;

	/** RegisterConfig()가 최소 한 번 호출되기 전에는 Tick하지 않는다. */
	bool bConfigured = false;
	bool bDeinitialized = false;

	UPROPERTY()
	TArray<TWeakObjectPtr<UKOVisionComponent>> RegisteredVisionComponents;

	TArray<FColor> CachedVisiblePixels;
	TArray<FColor> CachedExploredPixels;
	int32 CachedSizeX = 0;
	int32 CachedSizeY = 0;
};