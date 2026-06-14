#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOGhostPreview.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

UCLASS()
class KARON_API AKOGhostPreview : public AActor
{
	GENERATED_BODY()

public:
	AKOGhostPreview();

	void SetupFromBuildingClass(TSubclassOf<AActor> InBuildingClass);

	/** 에너지 커버리지 오버레이를 월드 중심/XY 크기로 표시(그리드 정렬, 회전 무관). */
	void ShowCoverageOverlay(const FVector& WorldCenter, const FVector2D& WorldSize);
	void HideCoverageOverlay();

private:
	void ClearPreviewMeshComponents();

	void AddPreviewMeshComponentFromTemplate(
		const UStaticMeshComponent* SourceMeshComponent
	);

private:
	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PreviewMeshComponents;

	// ─── 커버리지 오버레이(배치 프리뷰 전용) ──────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Coverage")
	TObjectPtr<UStaticMesh> CoveragePlaneMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Preview|Coverage")
	TObjectPtr<UMaterialInterface> CoverageMaterial;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CoverageMeshComponent;
};