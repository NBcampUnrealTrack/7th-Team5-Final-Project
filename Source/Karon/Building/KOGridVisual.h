#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOGridVisual.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

UCLASS()
class KARON_API AKOGridVisual : public AActor
{
	GENERATED_BODY()

public:
	AKOGridVisual();

protected:
	virtual void BeginPlay() override;

public:
	void BuildGridVisual();
	void SetGridVisible(bool bVisible);
	
	// 전력 범위
	void RefreshInstalledPowerCoverage(); // 설치된 발전기 전력 범위 갱신
	void ClearPowerCoverageCells(); // 전력 범위 지우기
	
private:
	void AddPowerCoverageArea( // 전력 범위 셀 추가
		const FIntPoint& Anchor,
		const FIntPoint& Size,
		int32 PowerRadius
	);

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> GridLineInstances;

	UPROPERTY(EditAnywhere, Category = "Grid Visual")
	TObjectPtr<UStaticMesh> LineMesh;

	UPROPERTY(EditAnywhere, Category = "Grid Visual")
	TObjectPtr<UMaterialInterface> LineMaterial;

	// 선 굻기
	UPROPERTY(EditAnywhere, Category = "Grid Visual")
	float LineThickness = 2.0f;
	
	UPROPERTY(EditAnywhere, Category = "Grid Visual")
	float ZOffset = 3.0f;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInstancedStaticMeshComponent> PowerCellInstances;

	UPROPERTY(EditAnywhere, Category = "Grid Visual|Power")
	TObjectPtr<UStaticMesh> PowerCellMesh;

	UPROPERTY(EditAnywhere, Category = "Grid Visual|Power")
	TObjectPtr<UMaterialInterface> PowerCellMaterial;

	UPROPERTY(EditAnywhere, Category = "Grid Visual|Power")
	float PowerCellZOffset = 1.5f;
};
