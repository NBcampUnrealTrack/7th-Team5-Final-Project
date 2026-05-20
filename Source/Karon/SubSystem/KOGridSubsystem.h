#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/KOGridCellData.h"
#include "KOGridSubSystem.generated.h"

UCLASS()
class KARON_API UKOGridSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

public:
	UFUNCTION(BlueprintCallable, Category = "Grid|Coordinate")
	FIntPoint WorldToGridPosition(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Grid|Coordinate")
	FVector GridToWorldPosition(const FIntPoint& GridLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Grid|Coordinate")
	FVector GetAreaCenterWorldPosition(FIntPoint AnchorLocation, FIntPoint AreaSize) const;
		
	UFUNCTION(BlueprintCallable, Category = "Grid|Setup")
	void BuildGridFromWorld();

	UFUNCTION(BlueprintCallable, Category = "Grid|Query")
	bool IsValidGridLocation(const FIntPoint& GridLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Grid|Query")
	bool CanBuildAt(const FIntPoint& GridLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Grid|Query")
	bool CanBuildArea(FIntPoint AnchorLocation, FIntPoint AreaSize, bool bCheckCollision = true) const;

	UFUNCTION(BlueprintCallable, Category = "Grid|Occupancy")
	void OccupyArea(FIntPoint AnchorLocation, FIntPoint AreaSize, AActor* OccupyingActor);

	// 특정 면적 점유 해제
	UFUNCTION(BlueprintCallable, Category = "Grid|Occupancy")
	void FreeArea(FIntPoint AnchorLocation, FIntPoint AreaSize);

	UFUNCTION(BlueprintCallable, Category = "Grid|Occupancy")
	void PlaceActorAt(const FIntPoint& GridLocation, AActor* OccupyingActor);

	UFUNCTION(BlueprintCallable, Category = "Grid|Occupancy")
	void RemoveActorAt(const FIntPoint& GridLocation);

	// 특정 셀을 건설 가능/불가능으로 설정(아직 사용 x)
	UFUNCTION(BlueprintCallable, Category = "Grid|Edit")
	void SetCellBuildable(const FIntPoint& GridLocation, bool bBuildable);

	// 디버그 박스 크기 계산
	UFUNCTION(BlueprintPure, Category = "Grid|Settings")
	float GetCellSize() const { return CellSize; }
	
	// 라인 디버깅
	UFUNCTION(BlueprintCallable, Category = "Grid|Debug")
	void DrawDebugGrid(float Duration = -1.0f) const;

private:
	int32 ToIndex(const FIntPoint& GridLocation) const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Settings")
	float CellSize = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Settings")
	FIntPoint GridDimensions = FIntPoint(10, 10);
	
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Settings")
	FVector GridOrigin = FVector::ZeroVector;
	
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Trace")
	float TraceHeight = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grid|Trace")
	TEnumAsByte<ECollisionChannel> GroundTraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, Category = "Grid|Collision")
	TEnumAsByte<ECollisionChannel> PlacementCollisionChannel = ECC_GameTraceChannel1;

	UPROPERTY()
	TArray<FKOGridCellData> GridData;
	
	// 그리드
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Debug")
	bool bAutoDrawDebugGridOnBeginPlay = true;

	UPROPERTY(EditDefaultsOnly, Category = "Grid|Debug")
	int32 DebugGridLineStep = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Grid|Debug")
	float DebugGridZOffset = 20.0f;
	
};
