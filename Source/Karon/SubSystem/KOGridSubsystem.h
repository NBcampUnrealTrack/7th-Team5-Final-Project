#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/KOGridCellData.h"
#include "KOGridSubsystem.generated.h"

USTRUCT()
struct FKOGridOccupiedArea
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FIntPoint Anchor = FIntPoint::ZeroValue;

	UPROPERTY()
	FIntPoint Size = FIntPoint(1, 1);
};

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

	UFUNCTION(BlueprintCallable, Category = "Grid|Query")
	AActor* GetOccupyingActorAt(const FIntPoint& GridLocation) const;

	UFUNCTION(BlueprintCallable, Category = "Grid|Query")
	bool TryGetOccupiedAreaForActor(
		AActor* OccupyingActor,
		FIntPoint& OutAnchor,
		FIntPoint& OutSize
	) const;
	
	UFUNCTION(BlueprintCallable, Category = "Grid|Occupancy")
	void OccupyArea(FIntPoint AnchorLocation, FIntPoint AreaSize, AActor* OccupyingActor);
	
	UFUNCTION(BlueprintCallable, Category = "Grid|Occupancy")
	bool FreeAreaByActor(AActor* OccupyingActor);

	// 디버그 박스 크기 계산
	UFUNCTION(BlueprintPure, Category = "Grid|Settings")
	float GetCellSize() const { return CellSize; }
	
	// 라인 디버깅
	UFUNCTION(BlueprintCallable, Category = "Grid|Debug")
	void DrawDebugGrid(float Duration = -1.0f) const;
	
protected:
	void FreeArea(FIntPoint AnchorLocation, FIntPoint AreaSize);
	void PlaceActorAt(const FIntPoint& GridLocation, AActor* OccupyingActor);
	void RemoveActorAt(const FIntPoint& GridLocation);
	
private:
	int32 ToIndex(const FIntPoint& GridLocation) const;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Settings")
	float CellSize = 100.0f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Settings")
	FIntPoint GridDimensions = FIntPoint(10, 10);
	
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Settings")
	FVector GridOrigin = FVector(400.0f, 300.0f, 0.0f);
	
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Trace")
	float TraceHeight = 1000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Grid|Trace")
	TEnumAsByte<ECollisionChannel> GridGroundTraceChannel = ECC_Visibility;

	UPROPERTY(EditDefaultsOnly, Category = "Grid|Collision")
	TEnumAsByte<ECollisionChannel> PlacementCollisionChannel = ECC_GameTraceChannel1;

	UPROPERTY()
	TArray<FKOGridCellData> GridData;
	
	UPROPERTY()
	TMap<TObjectPtr<AActor>, FKOGridOccupiedArea> OccupiedAreaByActor;
	
	// 그리드
	UPROPERTY(EditDefaultsOnly, Category = "Grid|Debug")
	bool bAutoDrawDebugGridOnBeginPlay = true;

	UPROPERTY(EditDefaultsOnly, Category = "Grid|Debug")
	int32 DebugGridLineStep = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Grid|Debug")
	float DebugGridZOffset = 20.0f;
	
};
