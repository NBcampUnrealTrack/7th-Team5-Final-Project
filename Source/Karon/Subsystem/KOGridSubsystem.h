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
	
	void SetGridOrigin(const FVector& NewOrigin) { GridOrigin = NewOrigin; }

public:
	FIntPoint WorldToGridPosition(const FVector& WorldLocation) const;
	FVector GridToWorldPosition(const FIntPoint& GridLocation) const;
	FVector GetAreaCenterWorldPosition(FIntPoint AnchorLocation, FIntPoint AreaSize) const;
		
	void BuildGridFromWorld();

	bool IsValidGridLocation(const FIntPoint& GridLocation) const;
	bool CanBuildAt(const FIntPoint& GridLocation) const;
	bool CanBuildArea(FIntPoint AnchorLocation, FIntPoint AreaSize, bool bCheckCollision = true) const;
	AActor* GetOccupyingActorAt(const FIntPoint& GridLocation) const;
	bool TryGetOccupiedAreaForActor(
		AActor* OccupyingActor,
		FIntPoint& OutAnchor,
		FIntPoint& OutSize
	) const;
	
	void OccupyArea(FIntPoint AnchorLocation, FIntPoint AreaSize, AActor* OccupyingActor);
	bool FreeAreaByActor(AActor* OccupyingActor);

	// 디버그 박스 크기 계산
	float GetCellSize() const { return CellSize; }
	FIntPoint GetGridDimensions() const { return GridDimensions; }
	FVector GetGridOrigin() const { return GridOrigin; }
	
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
	float TraceHeight = 10000.0f;

	ECollisionChannel GridGroundTraceChannel = ECC_Visibility;
	ECollisionChannel PlacementCollisionChannel = ECC_GameTraceChannel1;

	UPROPERTY()
	TArray<FKOGridCellData> GridData;
	
	UPROPERTY()
	TMap<TObjectPtr<AActor>, FKOGridOccupiedArea> OccupiedAreaByActor;
};
