#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KOGridBuildComponent.generated.h"

class UKOBuildingDataAsset;
class UMaterialInterface;
class AKOGhostPreview;
class UMeshComponent;
class AKOBaseBuilding;

USTRUCT()
struct FKODestroyTargetOriginalMaterials
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UMeshComponent> MeshComponent;

	UPROPERTY()
	TArray<TObjectPtr<UMaterialInterface>> Materials;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KARON_API UKOGridBuildComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKOGridBuildComponent();

	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

public:
	// ─── 건물 건설 ────────────────────────────────────────────────────	
	UFUNCTION(BlueprintCallable, Category = "Build")
	void StartBuildModeByIndex(int32 BuildIndex);
	
	UFUNCTION(BlueprintCallable, Category = "Build")
	void StartBuildModeWithData(UKOBuildingDataAsset* BuildingData);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void RequestBuild();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void CancelBuildMode();

	// 사용 x (UI)
	UFUNCTION(BlueprintPure, Category = "Build")
	bool IsBuildMode() const { return bIsBuildMode; }
	
	// ─── 건물 파괴(해제) ────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Build")
	void StartDestroyMode();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void RequestDestroy();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void CancelDestroyMode();

	UFUNCTION(BlueprintPure, Category = "Build")
	bool IsDestroyMode() const { return bIsDestroyMode; }

protected:
	bool TraceFromScreenCenter(FHitResult& OutHit) const;
	
	// 고스트 프리뷰 - 건설 모드
	void UpdateGhostPreview();
	bool SpawnPreviewActor();
	void DestroyPreviewActor();
	
	// 머티리얼
	void SetPreviewActorBuildableState(bool bCanBuild);
	void ApplyGhostMaterial(AActor* TargetActor, UMaterialInterface* TargetMaterial) const;
	
	// 파괴 모드
	void UpdateDestroyTargetPreview();
	void SetDestroyTargetActor(AActor* NewTargetActor);
	void ClearDestroyTargetActor();
	void ApplyDestroyTargetMaterial(AActor* TargetActor);
	void RestoreDestroyTargetMaterial();

protected:	
	// 여러 건물 선택용 DataAsset 목록
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Data")
	TArray<TObjectPtr<UKOBuildingDataAsset>> BuildOptions;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Ghost")
	TObjectPtr<UMaterialInterface> BuildableGhostMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Ghost")
	TObjectPtr<UMaterialInterface> NotBuildableGhostMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Ghost")
	TSubclassOf<AKOGhostPreview> PreviewActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Trace")
	float TraceDistance = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Trace")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel = ECC_Visibility;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Destroy")
	TObjectPtr<UMaterialInterface> DestroyTargetMaterial;
	
	// 충돌 검사
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Placement")
	bool bCheckPlacementCollision = true;

	// 연속적인 건설 모드(연속 설치)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Placement")
	bool bKeepBuildModeAfterPlacement = true;

private:
	UPROPERTY()
	TObjectPtr<AKOGhostPreview> CurrentPreviewActor;

	UPROPERTY()
	TObjectPtr<UKOBuildingDataAsset> CurrentBuildingData;
	
	UPROPERTY()
	TWeakObjectPtr<AActor> CurrentDestroyTargetActor;

	UPROPERTY()
	TArray<FKODestroyTargetOriginalMaterials> DestroyTargetOriginalMaterials;

	FIntPoint CurrentAnchor = FIntPoint::ZeroValue;

	FIntPoint CurrentBuildingSize = FIntPoint(1, 1);

	bool bIsBuildMode = false; // 건설
	bool bIsDestroyMode = false; // 파괴(해제)
	bool bCurrentPlacementValid = false; // 설치 가능 여부
	
	// 이전 상태를 저장한 적이 있는가?
	bool bHasLastPreviewBuildableState = false;
	// 마지막 설치 가능 상태
	bool bLastPreviewBuildableState = false;
};