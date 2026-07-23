#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOFogManager.generated.h"

class UTextureRenderTarget2D;
class UMaterialInterface;

/**
 * AKOFogManager
 * 레벨에 배치하는 Fog of War 설정 캐리어. 실제 Tick/가시성 판정 로직은 더 이상 이 액터가
 * 아니라 LocalPlayer 수명인 UKOFogManagerSubsystem이 담당하며, 이 액터는 BeginPlay에서
 * 레벨별 설정(RenderTarget/Material/MapSize/MapOrigin)을 그 서브시스템에 주입만 한다.
 */
UCLASS()
class KARON_API AKOFogManager : public AActor
{
	GENERATED_BODY()

public:
	AKOFogManager();

	virtual void BeginPlay() override;

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
};