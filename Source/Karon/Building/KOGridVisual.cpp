#include "KOGridVisual.h"

#include "Building/KOBaseBuilding.h"
#include "Component/Factory/KOEnergyProducerComponent.h"
#include "Subsystem/KOGridSubsystem.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Data/KODataTableTypes.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

AKOGridVisual::AKOGridVisual()
{
	PrimaryActorTick.bCanEverTick = false;

	GridLineInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("GridLineInstances"));
	SetRootComponent(GridLineInstances);
	GridLineInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GridLineInstances->SetCastShadow(false);
	
	PowerCellInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PowerCellInstances"));
	PowerCellInstances->SetupAttachment(RootComponent);
	PowerCellInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PowerCellInstances->SetCastShadow(false);

	// 시작할 때는 숨김
	SetActorHiddenInGame(true);
}

void AKOGridVisual::BeginPlay()
{
	Super::BeginPlay();
	
	BuildGridVisual();

	// 게임 시작 시 기본 숨김
	SetGridVisible(false);
}

void AKOGridVisual::BuildGridVisual()
{
	if (LineMesh)
	{
		GridLineInstances->SetStaticMesh(LineMesh);
	}
	
	if (LineMaterial)
	{
		GridLineInstances->SetMaterial(0, LineMaterial);
	}

	GridLineInstances->ClearInstances();
	
	if (PowerCellMesh)
	{
		PowerCellInstances->SetStaticMesh(PowerCellMesh);
	}

	if (PowerCellMaterial)
	{
		PowerCellInstances->SetMaterial(0, PowerCellMaterial);
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UKOGridSubsystem* GridSub = World->GetSubsystem<UKOGridSubsystem>();
	if (!GridSub)
	{
		return;
	}

	const float CellSize = GridSub->GetCellSize(); // 칸의 크기
	const FIntPoint GridDimensions = GridSub->GetGridDimensions(); // 가로,세로 칸 수
	const FVector GridOrigin = GridSub->GetGridOrigin(); // 그리드 시작 위치

	const float MinX = GridOrigin.X;
	const float MinY = GridOrigin.Y;
	const float MaxX = GridOrigin.X + GridDimensions.X * CellSize;
	const float MaxY = GridOrigin.Y + GridDimensions.Y * CellSize;
	const float Z = GridOrigin.Z + ZOffset;

	// 세로선
	for (int32 X = 0; X <= GridDimensions.X; ++X)
	{
		const float WorldX = GridOrigin.X + X * CellSize;

		const FVector Location(
			WorldX,
			(MinY + MaxY) * 0.5f,
			Z
		);

		const FVector Scale(
			LineThickness / 100.0f,
			(MaxY - MinY) / 100.0f,
			0.005f
		);

		GridLineInstances->AddInstance(
			FTransform(
				FRotator::ZeroRotator,
				Location,
				Scale
			)
		);
	}

	// 가로선
	for (int32 Y = 0; Y <= GridDimensions.Y; ++Y)
	{
		const float WorldY = GridOrigin.Y + Y * CellSize;

		const FVector Location(
			(MinX + MaxX) * 0.5f,
			WorldY,
			Z
		);

		const FVector Scale(
			(MaxX - MinX) / 100.0f,
			LineThickness / 100.0f,
			0.005f
		);

		GridLineInstances->AddInstance(
			FTransform(
				FRotator::ZeroRotator,
				Location,
				Scale
			)
		);
	}
}

void AKOGridVisual::SetGridVisible(bool bVisible)
{
	SetActorHiddenInGame(!bVisible);
	SetActorEnableCollision(false);
}

void AKOGridVisual::RefreshInstalledPowerCoverage()
{
	ClearPowerCoverageCells();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UKOGridSubsystem* GridSub = World->GetSubsystem<UKOGridSubsystem>();
	if (!GridSub)
	{
		return;
	}

	UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
	if (!LoadSub)
	{
		return;
	}

	for (TActorIterator<AKOBaseBuilding> It(World); It; ++It)
	{
		AKOBaseBuilding* Building = *It;
		if (!IsValid(Building))
		{
			continue;
		}

		// 발전기 컴포넌트가 있는 건물만 전력 범위 표시
		UKOEnergyProducerComponent* Producer =
			Building->FindComponentByClass<UKOEnergyProducerComponent>();

		if (!Producer)
		{
			continue;
		}

		const FName FactoryId = Building->GetFactoryId();
		if (FactoryId.IsNone())
		{
			continue;
		}

		const FKOFactoryRow* FactoryRow = LoadSub->FindFactoryRow(FactoryId);
		if (!FactoryRow)
		{
			continue;
		}

		const int32 PowerRadius = FactoryRow->EnergyCoverageRadius;
		if (PowerRadius <= 0)
		{
			continue;
		}

		FIntPoint OccupiedAnchor;
		FIntPoint OccupiedSize;

		if (!GridSub->TryGetOccupiedAreaForActor(
			Building,
			OccupiedAnchor,
			OccupiedSize
		))
		{
			continue;
		}

		AddPowerCoverageArea(
			OccupiedAnchor,
			OccupiedSize,
			PowerRadius
		);
	}
}

void AKOGridVisual::AddPowerCoverageArea(const FIntPoint& Anchor, const FIntPoint& Size, int32 PowerRadius)
{
	if (PowerRadius <= 0)
	{
		return;
	}

	if (!PowerCellInstances || !PowerCellMesh)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UKOGridSubsystem* GridSub = World->GetSubsystem<UKOGridSubsystem>();
	if (!GridSub)
	{
		return;
	}

	const float CellSize = GridSub->GetCellSize();

	const FIntPoint CoverageAnchor(
		Anchor.X - PowerRadius,
		Anchor.Y - PowerRadius
	);

	const FIntPoint CoverageSize(
		Size.X + PowerRadius * 2,
		Size.Y + PowerRadius * 2
	);

	for (int32 Y = 0; Y < CoverageSize.Y; ++Y)
	{
		for (int32 X = 0; X < CoverageSize.X; ++X)
		{
			const FIntPoint TargetGrid(
				CoverageAnchor.X + X,
				CoverageAnchor.Y + Y
			);

			if (!GridSub->IsValidGridLocation(TargetGrid))
			{
				continue;
			}

			FVector CellCenter = GridSub->GridToWorldPosition(TargetGrid);
			CellCenter.Z += PowerCellZOffset;

			const FVector Scale(
				CellSize / 100.0f,
				CellSize / 100.0f,
				1.0f
			);

			PowerCellInstances->AddInstance(
				FTransform(
					FRotator::ZeroRotator,
					CellCenter,
					Scale
				)
			);
		}
	}
}

void AKOGridVisual::ClearPowerCoverageCells()
{
	if (PowerCellInstances)
	{
		PowerCellInstances->ClearInstances();
	}
}
