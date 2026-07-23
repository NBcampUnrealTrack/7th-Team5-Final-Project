#include "KOConveyorFlowResolver.h"

#include "Building/Conveyor/KOConveyorBelt.h"
#include "Subsystem/KOGridSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystem/KOItemPortTypes.h"

FIntPoint FKOConveyorFlowResolver::WorldDirToGridStep(const FVector& Dir)
{
	if (FMath::Abs(Dir.X) >= FMath::Abs(Dir.Y))
	{
		return FIntPoint(Dir.X >= 0.0f ? 1 : -1, 0);
	}

	return FIntPoint(0, Dir.Y >= 0.0f ? 1 : -1);
}

FVector FKOConveyorFlowResolver::GridStepToWorldDir(const FIntPoint& Step)
{
	return FVector(static_cast<float>(Step.X), static_cast<float>(Step.Y), 0.0f).GetSafeNormal();
}

FIntPoint FKOConveyorFlowResolver::NegateStep(const FIntPoint& Step)
{
	return FIntPoint(-Step.X, -Step.Y);
}

int32 FKOConveyorFlowResolver::ClassifyNeighbor(
	const UWorld* World,
	const FIntPoint& MyCell,
	const FIntPoint& NeighborCell
)
{
	const UKOGridSubsystem* Grid = World
		? World->GetSubsystem<UKOGridSubsystem>()
		: nullptr;

	if (!Grid)
	{
		return 0;
	}

	AActor* Actor = Grid->GetOccupyingActorAt(NeighborCell);

	if (!Actor)
	{
		return 0;
	}

	if (const AKOConveyorBelt* Belt = Cast<AKOConveyorBelt>(Actor))
	{
		if (Belt->OutputsToCell(MyCell)) { return +1; } // 이웃 벨트가 나를 향해 출력 → 업스트림

		if (Belt->InputsFromCell(MyCell)) { return -1; } // 이웃 벨트가 나에게서 입력 → 다운스트림

		return 0;
	}

	// 머신: 단방향 포트만 있으면 방향 확정, 양쪽(Processor) 또는 없음이면 모호.
	const bool bHasSource = Cast<IKOItemSource>(Actor) != nullptr;
	const bool bHasSink = Cast<IKOItemSink>(Actor) != nullptr;

	bool bComponentHasSource = false;
	bool bComponentHasSink = false;

	TInlineComponentArray<UActorComponent*> Components(Actor);
	for (UActorComponent* Component : Components)
	{
		if (!Component)
		{
			continue;
		}

		if (Cast<IKOItemSource>(Component))
		{
			bComponentHasSource = true;
		}

		if (Cast<IKOItemSink>(Component))
		{
			bComponentHasSink = true;
		}
	}

	const bool bFinalHasSource = bHasSource || bComponentHasSource;
	const bool bFinalHasSink = bHasSink || bComponentHasSink;

	if (bFinalHasSource && !bFinalHasSink)
	{
		return +1; // 출력만 있음 → 나에게 공급
	}

	if (bFinalHasSink && !bFinalHasSource)
	{
		return -1; // 입력만 있음 → 내가 공급
	}

	return 0;
}

bool FKOConveyorFlowResolver::TryResolveStraightReverse(
	const UWorld* World,
	const FIntPoint& MyCell,
	const FIntPoint& Forward,
	bool& OutReverse
)
{
	const FIntPoint FrontCell = MyCell + Forward;
	const FIntPoint BackCell = MyCell - Forward;

	const int32 RoleFront = ClassifyNeighbor(World, MyCell, FrontCell);
	const int32 RoleBack = ClassifyNeighbor(World, MyCell, BackCell);

	// 정방향 유지: 뒤가 업스트림이거나 앞이 다운스트림이면 그대로.
	const bool bWantForward = (RoleBack > 0) || (RoleFront < 0);

	// 역방향 반전: 앞이 업스트림이거나 뒤가 다운스트림이면 반전.
	const bool bWantReverse = (RoleFront > 0) || (RoleBack < 0);

	if (bWantForward && !bWantReverse) { OutReverse = false; return true; }

	if (bWantReverse && !bWantForward) { OutReverse = true; return true; }

	return false;
}

bool FKOConveyorFlowResolver::TryResolveCornerFlip(
	const UWorld* World,
	const FIntPoint& MyCell,
	const FIntPoint& Forward,
	const FIntPoint& Side,
	bool& OutFlip
)
{
	const FIntPoint CellA = MyCell + Forward;
	const FIntPoint CellB = MyCell + Side;

	const int32 RoleA = ClassifyNeighbor(World, MyCell, CellA);
	const int32 RoleB = ClassifyNeighbor(World, MyCell, CellB);

	// flip=false:
	// 입구 = +Side, 출구 = +Forward
	const bool bWantFalse = (RoleB > 0) || (RoleA < 0);

	// flip=true:
	// 입구 = +Forward, 출구 = +Side
	const bool bWantTrue = (RoleA > 0) || (RoleB < 0);

	if (bWantFalse && !bWantTrue){ OutFlip = false; return true; }

	if (bWantTrue && !bWantFalse){ OutFlip = true; return true;	}

	return false;
}