#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KOGridBuildComponent.generated.h"

class UKOBuildingDataAsset;
class UMaterialInterface;
class AKOGhostPreview;

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
	UFUNCTION(BlueprintCallable, Category = "Build") 
	void StartAssignedBuildMode();
	
	UFUNCTION(BlueprintCallable, Category = "Build")
	void StartBuildModeWithData(UKOBuildingDataAsset* BuildingData);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void RequestBuild();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void CancelBuildMode();

	// 사용 x (UI)
	UFUNCTION(BlueprintPure, Category = "Build")
	bool IsBuildMode() const { return bIsBuildMode; }

protected:
	bool TraceFromScreenCenter(FHitResult& OutHit) const;

	APlayerController* GetOwningPlayerController() const;

	void UpdateGhostPreview();
	bool SpawnPreviewActor();
	void DestroyPreviewActor();
	void SetPreviewActorBuildableState(bool bCanBuild);
	void ApplyGhostMaterial(AActor* TargetActor, UMaterialInterface* TargetMaterial) const;

protected:
	// 단일 건물 테스트
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Data")
	TObjectPtr<UKOBuildingDataAsset> DefaultBuildingData;
	
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

	FIntPoint CurrentAnchor = FIntPoint::ZeroValue;

	FIntPoint CurrentBuildingSize = FIntPoint(1, 1);

	bool bIsBuildMode = false;
	bool bCurrentPlacementValid = false;
	
	// 이전 상태를 저장한 적이 있는가?
	bool bHasLastPreviewBuildableState = false;
	// 마지막 설치 가능 상태
	bool bLastPreviewBuildableState = false;
};