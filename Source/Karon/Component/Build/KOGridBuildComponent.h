#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utility/Messaging/KOGMSInterface.h"

#include "KOGridBuildComponent.generated.h"

struct FKOFactoryRow;

class UMaterialInterface;
class AKOGhostPreview;
class UMeshComponent;
class AKOBaseBuilding;
class UKOInventoryComponent;

UENUM(BlueprintType)
enum class EKOGridBuildMode : uint8
{
	None	UMETA(DisplayName = "None"),
	BuildMenu	UMETA(DisplayName = "Build Menu"),
	Placing		UMETA(DisplayName = "Placing"),
	Destroying	UMETA(DisplayName = "Destroying")
};

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
class KARON_API UKOGridBuildComponent : public UActorComponent, public IKOGMSInterface
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
	void StartBuildModeWithId(FName FactoryId);

	UFUNCTION(BlueprintCallable, Category = "Build")
	void RequestBuild();
	
	// ─── 건물 파괴(해제) ────────────────────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "Build")
	void StartDestroyMode();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void RequestDestroy();
	
	// ─── 건설 모드 ────────────────────────────────────────────────────	
	UFUNCTION(BlueprintCallable, Category = "Build")
	void EnterBuildMenuMode();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void ExitBuildMenuMode();

	UFUNCTION(BlueprintPure, Category = "Build")
	bool IsBuildMenuMode() const { return CurrentMode == EKOGridBuildMode::BuildMenu; }

	UFUNCTION(BlueprintPure, Category = "Build")
	bool IsBuildMode() const { return CurrentMode == EKOGridBuildMode::Placing; }

	UFUNCTION(BlueprintPure, Category = "Build")
	bool IsDestroyMode() const { return CurrentMode == EKOGridBuildMode::Destroying; }

	UFUNCTION(BlueprintPure, Category = "Build")
	bool IsBuildSystemActive() const
	{
		return CurrentMode == EKOGridBuildMode::BuildMenu ||
			CurrentMode == EKOGridBuildMode::Placing ||
			CurrentMode == EKOGridBuildMode::Destroying;
	}
	
	UFUNCTION(BlueprintPure, Category = "Build")
	EKOGridBuildMode GetCurrentMode() const { return CurrentMode; }
	
	// ─── 건설 모드 해제 ────────────────────────────────────────────────────	
	UFUNCTION(BlueprintCallable, Category = "Build")
	void CancelCurrentMode();
	
	UFUNCTION(BlueprintCallable, Category = "Build")
	void CancelBuildMode();
	
	UFUNCTION(BlueprintCallable, Category = "Build")
	void CancelDestroyMode();	
	
	UFUNCTION(BlueprintCallable, Category = "Build|Placement")
	void RotatePlacementPreview(int32 Direction);

protected:
	bool TraceFromScreenCenter(
		FHitResult& OutHit,
		ECollisionChannel TraceChannel
	) const;
	
	// 고스트 프리뷰 - 건설 모드
	void UpdateGhostPreview();
	bool SpawnPreviewActor();
	void DestroyPreviewActor();
	void ClearPlacementState();
	
	// 머티리얼
	void SetPreviewActorBuildableState(bool bCanBuild);
	void ApplyGhostMaterial(AActor* TargetActor, UMaterialInterface* TargetMaterial) const;
	
	// 파괴 모드
	void UpdateDestroyTargetPreview();
	void SetDestroyTargetActor(AActor* NewTargetActor);
	void ClearDestroyTargetActor();
	void ApplyDestroyTargetMaterial(AActor* TargetActor);
	void RestoreDestroyTargetMaterial();
	
	UKOInventoryComponent* GetInventoryComponent() const;
	
private:
	void SetCurrentMode(EKOGridBuildMode NewMode);
	FRotator GetPlacementRotation() const;
	FIntPoint GetRotatedBuildingSize() const;

	/** 현재 배치 중인 건물 클래스가 코너 형태 컨베이어 벨트인지 CDO로 판정. */
	bool IsCurrentBuildingCornerBelt() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Ghost")
	TObjectPtr<UMaterialInterface> BuildableGhostMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Ghost")
	TObjectPtr<UMaterialInterface> NotBuildableGhostMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Ghost")
	TSubclassOf<AKOGhostPreview> PreviewActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Trace")
	float TraceDistance = 1000.0f;

	ECollisionChannel BuildTraceChannel = ECC_Visibility;
	ECollisionChannel DestroyTraceChannel  = ECC_Visibility;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Destroy")
	TObjectPtr<UMaterialInterface> DestroyTargetMaterial;
	
	// 충돌 검사
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Placement")
	bool bCheckPlacementCollision = true;

	// 연속적인 건설 모드(연속 설치)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Build|Placement")
	bool bKeepBuildModeAfterPlacement = true;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Build", meta = (AllowPrivateAccess = "true"))
	EKOGridBuildMode CurrentMode = EKOGridBuildMode::None;

private:
	UPROPERTY()
	TObjectPtr<AKOGhostPreview> CurrentPreviewActor;

	FName CurrentFactoryId = NAME_None;
	const FKOFactoryRow* CurrentFactoryRow = nullptr;
	
	TWeakObjectPtr<UClass> CurrentBuildingClass;
	TWeakObjectPtr<AActor> CurrentDestroyTargetActor;

	UPROPERTY()
	TArray<FKODestroyTargetOriginalMaterials> DestroyTargetOriginalMaterials;

	FIntPoint CurrentAnchor = FIntPoint::ZeroValue;

	FIntPoint BaseBuildingSize = FIntPoint(1, 1); // 원본 점유 크기
	FIntPoint CurrentBuildingSize = FIntPoint(1, 1); // 회전 점유 크기
	
	int32 CurrentRotationStep = 0;
	// 0 = 0도
	// 1 = 90도
	// 2 = 180도
	// 3 = 270도

	// 코너 벨트 흐름 반전 상태. yaw 가 4단계 경계(3↔0)를 넘을 때마다 토글 → 4 yaw × 2 flip = 8방향.
	bool bCornerFlipPlacement = false;

	bool bCurrentPlacementValid = false; // 설치 가능 여부
	
	// 이전 상태를 저장한 적이 있는가?
	bool bHasLastPreviewBuildableState = false;
	// 마지막 설치 가능 상태
	bool bLastPreviewBuildableState = false;
};
