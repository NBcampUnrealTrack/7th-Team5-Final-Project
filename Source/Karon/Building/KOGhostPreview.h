#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOGhostPreview.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class UMeshComponent;
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
	
	// 컨베이어 벨트 방향 표시
	void ShowDirectionArrow(float AdditionalYaw = 0.0f);
	void HideDirectionArrow();

private:
	void ClearPreviewMeshComponents();

	void AddPreviewMeshComponentFromTemplate(const UMeshComponent* SourceMeshComponent);

private:
	UPROPERTY()
	TArray<TObjectPtr<UMeshComponent>> PreviewMeshComponents;

	// ─── 커버리지 오버레이(배치 프리뷰 전용) ──────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Coverage")
	TObjectPtr<UStaticMesh> CoveragePlaneMesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Coverage")
	TObjectPtr<UMaterialInterface> CoverageMaterial;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CoverageMeshComponent;
	
	// ─── 방향 표시 화살표(컨베이어 프리뷰 전용) ──────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Direction")
	TObjectPtr<UStaticMesh> DirectionArrowMesh;
	
	UPROPERTY(EditDefaultsOnly, Category = "Preview|Direction")
	TObjectPtr<UMaterialInterface> DirectionArrowMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Preview|Direction")
	FVector DirectionArrowRelativeLocation = FVector(0.0f, 0.0f, 20.0f);

	UPROPERTY(EditDefaultsOnly, Category = "Preview|Direction")
	FRotator DirectionArrowRelativeRotation = FRotator::ZeroRotator;

	UPROPERTY(EditDefaultsOnly, Category = "Preview|Direction")
	FVector DirectionArrowRelativeScale = FVector(0.8f, 0.8f, 0.8f);

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> DirectionArrowComponent;
};