#include "KOGridSubsystem.h"
#include "Engine/World.h"

namespace
{
	template<typename FuncType>
	void ForEachGridInArea(FIntPoint AnchorLocation, FIntPoint AreaSize, FuncType&& Func)
	{
		for (int32 Y = 0; Y < AreaSize.Y; ++Y)
		{
			for (int32 X = 0; X < AreaSize.X; ++X)
			{
				const FIntPoint TargetGrid(
					AnchorLocation.X + X,
					AnchorLocation.Y + Y
				);

				Func(TargetGrid);
			}
		}
	}
}

void UKOGridSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	BuildGridFromWorld(); // 그리드 데이터 생성
}

void UKOGridSubsystem::BuildGridFromWorld()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	
	if (GridDimensions.X <= 0 || GridDimensions.Y <= 0 || CellSize <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Grid] Grid 설정값이 잘못되었습니다. Dimensions(%d, %d), CellSize(%f)"),
			GridDimensions.X,
			GridDimensions.Y,
			CellSize
		);

		GridData.Empty();
		return;
	}

	const int32 TotalCells = GridDimensions.X * GridDimensions.Y;

	GridData.Empty();
	OccupiedAreaByActor.Empty();
	
	GridData.SetNum(TotalCells);

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;

	for (int32 Y = 0; Y < GridDimensions.Y; ++Y)
	{
		for (int32 X = 0; X < GridDimensions.X; ++X)
		{
			const float WorldX = GridOrigin.X + (X * CellSize) + (CellSize * 0.5f);
			const float WorldY = GridOrigin.Y + (Y * CellSize) + (CellSize * 0.5f);

			const FVector TraceStart(WorldX, WorldY, GridOrigin.Z + TraceHeight);
			const FVector TraceEnd(WorldX, WorldY, GridOrigin.Z - TraceHeight);

			FHitResult HitResult;

			const bool bHitGround = World->LineTraceSingleByChannel(
				HitResult,
				TraceStart,
				TraceEnd,
				GridGroundTraceChannel,
				QueryParams
			);

			const int32 Index = ToIndex(FIntPoint(X, Y));

			if (bHitGround)
			{
				GridData[Index].TerrainZ = HitResult.ImpactPoint.Z;
				GridData[Index].bIsBuildable = true;
			}
			else
			{
				GridData[Index].TerrainZ = 0.0f;
				GridData[Index].bIsBuildable = false;
			}
		}
	}
}

FIntPoint UKOGridSubsystem::WorldToGridPosition(const FVector& WorldLocation) const
{
	const int32 X = FMath::FloorToInt((WorldLocation.X - GridOrigin.X) / CellSize);
	const int32 Y = FMath::FloorToInt((WorldLocation.Y - GridOrigin.Y) / CellSize);

	return FIntPoint(X, Y);
}

FVector UKOGridSubsystem::GridToWorldPosition(const FIntPoint& GridLocation) const
{
	const float WorldX = GridOrigin.X + (GridLocation.X * CellSize) + (CellSize * 0.5f);
	const float WorldY = GridOrigin.Y + (GridLocation.Y * CellSize) + (CellSize * 0.5f);

	float CachedZ = 0.0f;

	if (IsValidGridLocation(GridLocation))
	{
		const int32 Index = ToIndex(GridLocation);
		CachedZ = GridData[Index].TerrainZ;
	}

	return FVector(WorldX, WorldY, CachedZ);
}

FVector UKOGridSubsystem::GetAreaCenterWorldPosition(FIntPoint AnchorLocation, FIntPoint AreaSize) const
{
	const float WorldX = GridOrigin.X + (AnchorLocation.X * CellSize) + (AreaSize.X * CellSize * 0.5f);
	const float WorldY = GridOrigin.Y + (AnchorLocation.Y * CellSize) + (AreaSize.Y * CellSize * 0.5f);

	float CachedZ = 0.0f;

	if (IsValidGridLocation(AnchorLocation))
	{
		const int32 Index = ToIndex(AnchorLocation);
		CachedZ = GridData[Index].TerrainZ;
	}

	return FVector(WorldX, WorldY, CachedZ);
}

bool UKOGridSubsystem::CanBuildArea(FIntPoint AnchorLocation, FIntPoint AreaSize, bool bCheckCollision) const
{
	if (AreaSize.X <= 0 || AreaSize.Y <= 0)
	{
		return false;
	}

	for (int32 Y = 0; Y < AreaSize.Y; ++Y)
	{
		for (int32 X = 0; X < AreaSize.X; ++X)
		{
			const FIntPoint TargetGrid(
				AnchorLocation.X + X,
				AnchorLocation.Y + Y
			);

			if (!CanBuildAt(TargetGrid))
			{
				return false;
			}
		}
	}

	if (!bCheckCollision)
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FVector CenterLocation = GetAreaCenterWorldPosition(AnchorLocation, AreaSize);
	CenterLocation.Z += 50.0f;

	const FVector BoxExtent(
		AreaSize.X * CellSize * 0.45f,
		AreaSize.Y * CellSize * 0.45f,
		50.0f
	);

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;

	const bool bIsOverlapping = World->OverlapAnyTestByChannel(
		CenterLocation,
		FQuat::Identity,
		PlacementCollisionChannel,
		FCollisionShape::MakeBox(BoxExtent),
		QueryParams
	);

	return !bIsOverlapping;
}

AActor* UKOGridSubsystem::GetOccupyingActorAt(const FIntPoint& GridLocation) const
{
	if (!IsValidGridLocation(GridLocation))
	{
		return nullptr;
	}

	const int32 Index = ToIndex(GridLocation);

	if (!GridData.IsValidIndex(Index))
	{
		return nullptr;
	}

	return GridData[Index].OccupyingActor.Get();
}

bool UKOGridSubsystem::TryGetOccupiedAreaForActor(
	AActor* OccupyingActor,
	FIntPoint& OutAnchor,
	FIntPoint& OutSize
) const
{
	if (!OccupyingActor)
	{
		return false;
	}

	const FKOGridOccupiedArea* FoundArea = OccupiedAreaByActor.Find(OccupyingActor);

	if (!FoundArea)
	{
		return false;
	}

	OutAnchor = FoundArea->Anchor;
	OutSize = FoundArea->Size;

	return true;
}

void UKOGridSubsystem::OccupyArea(FIntPoint AnchorLocation, FIntPoint AreaSize, AActor* OccupyingActor)
{
	if (!OccupyingActor || AreaSize.X <= 0 || AreaSize.Y <= 0)
	{
		return;
	}

	ForEachGridInArea(AnchorLocation, AreaSize, [this, OccupyingActor](const FIntPoint& TargetGrid)
	{
		PlaceActorAt(TargetGrid, OccupyingActor);
	});
	
	FKOGridOccupiedArea OccupiedArea;
	OccupiedArea.Anchor = AnchorLocation;
	OccupiedArea.Size = AreaSize;

	OccupiedAreaByActor.Add(OccupyingActor, OccupiedArea);
}

void UKOGridSubsystem::FreeArea(FIntPoint AnchorLocation, FIntPoint AreaSize)
{
	if (AreaSize.X <= 0 || AreaSize.Y <= 0)
	{
		return;
	}
	
	ForEachGridInArea(AnchorLocation, AreaSize, [this](const FIntPoint& TargetGrid)
	{
		RemoveActorAt(TargetGrid);
	});
}

bool UKOGridSubsystem::FreeAreaByActor(AActor* OccupyingActor)
{
	if (!OccupyingActor)
	{
		return false;
	}

	FKOGridOccupiedArea OccupiedArea;

	if (!OccupiedAreaByActor.RemoveAndCopyValue(OccupyingActor, OccupiedArea))
	{
		return false;
	}
	
	FreeArea(OccupiedArea.Anchor, OccupiedArea.Size);

	return true;
}

bool UKOGridSubsystem::CanBuildAt(const FIntPoint& GridLocation) const
{
	if (!IsValidGridLocation(GridLocation))
	{
		return false;
	}

	const int32 Index = ToIndex(GridLocation);
	const FKOGridCellData& Cell = GridData[Index];

	if (!Cell.bIsBuildable || Cell.OccupyingActor.IsValid())
	{
		return false;
	}

	return true;
}

void UKOGridSubsystem::PlaceActorAt(const FIntPoint& GridLocation, AActor* OccupyingActor)
{
	if (!IsValidGridLocation(GridLocation))
	{
		return;
	}

	if (!OccupyingActor)
	{
		return;
	}

	const int32 Index = ToIndex(GridLocation);

	GridData[Index].OccupyingActor = OccupyingActor;
}

void UKOGridSubsystem::RemoveActorAt(const FIntPoint& GridLocation)
{
	if (!IsValidGridLocation(GridLocation))
	{
		return;
	}

	const int32 Index = ToIndex(GridLocation);

	GridData[Index].OccupyingActor.Reset();
}

bool UKOGridSubsystem::IsValidGridLocation(const FIntPoint& GridLocation) const
{
	if (GridLocation.X < 0 ||
		GridLocation.X >= GridDimensions.X ||
		GridLocation.Y < 0 ||
		GridLocation.Y >= GridDimensions.Y)
	{
		return false;
	}

	const int32 Index = ToIndex(GridLocation);
	return GridData.IsValidIndex(Index);
}

int32 UKOGridSubsystem::ToIndex(const FIntPoint& GridLocation) const
{
	return GridLocation.Y * GridDimensions.X + GridLocation.X;
}