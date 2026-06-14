#include "Component/Build/KOGridBuildComponent.h"
#include "Data/KODataTableTypes.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "SubSystem/KOGridSubSystem.h"
#include "Building/KOBaseBuilding.h"
#include "Building/KOGhostPreview.h"
#include "Building/Conveyor/KOConveyorBelt.h"
#include "DrawDebugHelpers.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "HAL/IConsoleManager.h"

#include "Components/MeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"


#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "StructUtils/InstancedStruct.h"
#include "Utility/Messaging/KOMessageTypes.h"

#include "UI/KOUISubsystem.h"
#include "UI/Interaction/KOBeltConnectWidget.h"
#include "Component/Factory/KOFactoryProcessorComponent.h"
#include "CommonActivatableWidget.h"

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)

static TAutoConsoleVariable<int32> CVarKODrawBuildTrace(
	TEXT("ko.DrawBuildTrace"),
	0,
	TEXT("화면 중앙 라인트레이스 디버그 표시 여부. 0: Off, 1: On"),
	ECVF_Cheat
);

static TAutoConsoleVariable<int32> CVarKODrawBuildOccupiedCells(
	TEXT("ko.DrawBuildCells"),
	0,
	TEXT("건물이 점유할 그리드 셀 디버그 박스 표시 여부. 0: Off, 1: On"),
	ECVF_Cheat
);

#endif

DEFINE_LOG_CATEGORY_STATIC(LogKOBuild, Log, All);

UKOGridBuildComponent::UKOGridBuildComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// 건설 모드 or 파괴 모드가 켜졌을 때만 tick 켬
	PrimaryComponentTick.SetTickFunctionEnable(false);
	
	// 기본 고스트 프리뷰 클래스 지정
	PreviewActorClass = AKOGhostPreview::StaticClass();
}

void UKOGridBuildComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction
)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (CurrentMode)
	{
	case EKOGridBuildMode::Placing:
		UpdateGhostPreview();
		break;

	case EKOGridBuildMode::Destroying:
		UpdateDestroyTargetPreview();
		break;

	case EKOGridBuildMode::BuildMenu:
	case EKOGridBuildMode::None:
	default:
		break;
	}
}

void UKOGridBuildComponent::EnterBuildMenuMode()
{
	if (CurrentMode == EKOGridBuildMode::BuildMenu)
	{
		return;
	}

	ExitBuildMenuMode();

	SetCurrentMode(EKOGridBuildMode::BuildMenu);
	SetComponentTickEnabled(false);

	UE_LOG(LogKOBuild, Log, TEXT("[Build] 건설 메뉴 모드 시작"));
}

void UKOGridBuildComponent::ExitBuildMenuMode()
{
	if (CurrentMode == EKOGridBuildMode::Placing)
	{
		ClearPlacementState();
	}
	else if (CurrentMode == EKOGridBuildMode::Destroying)
	{
		ClearDestroyTargetActor();
	}

	SetCurrentMode(EKOGridBuildMode::None);
	SetComponentTickEnabled(false);

	UE_LOG(LogKOBuild, Log, TEXT("[Build] 건설 메뉴 모드 종료"));
}

void UKOGridBuildComponent::StartBuildModeWithId(FName FactoryId)
{
	if (!IsBuildSystemActive())
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] 건설 메뉴 모드가 아닐 때는 설치 모드로 들어갈 수 없습니다."));
		return;
	}
	
	if (FactoryId.IsNone())
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] FactoryId가 비어 있습니다."));
		return;
	}

	UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
	if (!LoadSub)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] KOLoadSubsystem을 찾을 수 없습니다."));
		return;
	}

	const FKOFactoryRow* Row = LoadSub->FindFactoryRow(FactoryId);
	if (!Row)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] 알 수 없는 FactoryId: %s"), *FactoryId.ToString());
		return;
	}

	UClass* BuildingClass = LoadSub->ResolveBuildingClass(FactoryId);
	if (!BuildingClass)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] BuildingClass 로드 실패. FactoryId=%s"), *FactoryId.ToString());
		return;
	}
	
	if (!BuildingClass->IsChildOf(AKOBaseBuilding::StaticClass()))
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] BuildingClass가 AKOBaseBuilding을 상속하지 않습니다. FactoryId=%s"),
			*FactoryId.ToString()
		);
		return;
	}

	if (!PreviewActorClass)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] PreviewActorClass가 설정되지 않았습니다."));
		return;
	}

	if (Row->GridSize.X <= 0 || Row->GridSize.Y <= 0)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] GridSize가 잘못되었습니다."));
		return;
	}

	CancelCurrentMode();

	CurrentFactoryId = FactoryId;
	CurrentFactoryRow = Row;
	CurrentBuildingClass = BuildingClass;
	BaseBuildingSize = Row->GridSize;
	CurrentRotationStep = 0;
	bCornerFlipPlacement = false;
	CurrentBuildingSize = GetRotatedBuildingSize();

	SetCurrentMode(EKOGridBuildMode::Placing);
	SetComponentTickEnabled(true);
	bCurrentPlacementValid = false;

	// 고스트 생성에 실패하면 건설 모드 취소
	if (!SpawnPreviewActor())
	{
		CancelBuildMode();
		return;
	}

	UpdateGhostPreview();
}

bool UKOGridBuildComponent::SpawnPreviewActor()
{
	if (CurrentPreviewActor)
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 공통 고스트 프리뷰 Actor를 스폰
	AKOGhostPreview* PreviewActor = World->SpawnActor<AKOGhostPreview>(
		PreviewActorClass,
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!PreviewActor)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] PreviewActor 생성 실패"));
		return false;
	}
	
	// 건물 BP의 StaticMesh를 읽어서 고스트 Actor에 복사
	CurrentPreviewActor = PreviewActor;
	PreviewActor->SetupFromBuildingClass(CurrentBuildingClass.Get());
	
	SetPreviewActorBuildableState(false);
	CurrentPreviewActor->SetActorHiddenInGame(false); // 고스트 생성
	
	return true;
}

void UKOGridBuildComponent::DestroyPreviewActor()
{
	if (CurrentPreviewActor)
	{
		CurrentPreviewActor->Destroy();
		CurrentPreviewActor = nullptr;
	}
	
	bHasLastPreviewBuildableState = false;
	bLastPreviewBuildableState = false;
}

void UKOGridBuildComponent::ClearPlacementState()
{
	bCurrentPlacementValid = false;

	DestroyPreviewActor();

	CurrentFactoryId = NAME_None;
	CurrentFactoryRow = nullptr;
	CurrentBuildingClass.Reset();
	CurrentAnchor = FIntPoint::ZeroValue;
	
	CurrentBuildingSize = FIntPoint(1, 1);
	BaseBuildingSize = FIntPoint(1, 1);
	CurrentRotationStep = 0;
	bCornerFlipPlacement = false;
}

void UKOGridBuildComponent::UpdateGhostPreview()
{
	if (!CurrentFactoryRow)
	{
		return;
	}

	if (!CurrentPreviewActor)
	{
		SpawnPreviewActor();
	}

	if (!CurrentPreviewActor)
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
		CurrentPreviewActor->SetActorHiddenInGame(true); // 고스트 숨김
		bCurrentPlacementValid = false; // 설치 불가
		return;
	}

	FHitResult HitResult;

	if (!TraceFromScreenCenter(HitResult, BuildTraceChannel))
	{
		CurrentPreviewActor->SetActorHiddenInGame(true);
		bCurrentPlacementValid = false;
		return;
	}

	CurrentPreviewActor->SetActorHiddenInGame(false);

	const FIntPoint HitGrid = GridSub->WorldToGridPosition(HitResult.ImpactPoint);

	// 홀수 크기는 HitGrid를 중심으로 배치하고,
	// 짝수 크기는 HitGrid를 Anchor로 사용하여 +X, +Y 방향으로 확장
	const int32 AnchorX = (CurrentBuildingSize.X % 2 == 0)
		? HitGrid.X : HitGrid.X - (CurrentBuildingSize.X / 2);

	const int32 AnchorY = (CurrentBuildingSize.Y % 2 == 0)
		? HitGrid.Y : HitGrid.Y - (CurrentBuildingSize.Y / 2);

	CurrentAnchor = FIntPoint(AnchorX, AnchorY);

	FVector PreviewLocation = GridSub->GetAreaCenterWorldPosition(
		CurrentAnchor,
		CurrentBuildingSize
	);
	PreviewLocation.Z += CurrentFactoryRow->PlacementZOffset;
	CurrentPreviewActor->SetActorLocationAndRotation(
		PreviewLocation,
		GetPlacementRotation()
	);

	// 설치할 수 있는지 검사
	const bool bCanBuild = GridSub->CanBuildArea(
		CurrentAnchor,
		CurrentBuildingSize,
		bCheckPlacementCollision
	);

	bCurrentPlacementValid = bCanBuild;

	SetPreviewActorBuildableState(bCanBuild);

	// 에너지 발전기면 공급 커버리지 면적을 초록 오버레이로 표시(프리뷰 중에만).
	if (CurrentFactoryRow->EnergyCoverageRadius > 0)
	{
		const int32 Radius = CurrentFactoryRow->EnergyCoverageRadius;
		const FIntPoint CoverageAnchor(
			CurrentAnchor.X - Radius,
			CurrentAnchor.Y - Radius
		);
		const FIntPoint CoverageSize(
			CurrentBuildingSize.X + 2 * Radius,
			CurrentBuildingSize.Y + 2 * Radius
		);

		const float CellSize = GridSub->GetCellSize();
		FVector CoverageCenter = GridSub->GetAreaCenterWorldPosition(CoverageAnchor, CoverageSize);
		CoverageCenter.Z = PreviewLocation.Z + 5.0f; // 바닥 z-파이팅 방지용 살짝 띄움

		CurrentPreviewActor->ShowCoverageOverlay(
			CoverageCenter,
			FVector2D(CoverageSize.X * CellSize, CoverageSize.Y * CellSize)
		);
	}
	else
	{
		CurrentPreviewActor->HideCoverageOverlay();
	}

	// 건물이 차지할 그리드 셀 디버그 표시
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (
		IsInGameThread() &&
		CVarKODrawBuildOccupiedCells.GetValueOnGameThread() != 0
	)
	{
		const float CellSize = GridSub->GetCellSize();

		const FVector CellExtent(
			CellSize * 0.48f,
			CellSize * 0.48f,
			5.0f
		);

		const FColor OccupiedColor = bCanBuild ? FColor::Cyan : FColor::Red;

		for (int32 Y = 0; Y < CurrentBuildingSize.Y; ++Y)
		{
			for (int32 X = 0; X < CurrentBuildingSize.X; ++X)
			{
				const FIntPoint TargetGrid(
					CurrentAnchor.X + X,
					CurrentAnchor.Y + Y
				);

				FVector CellCenter = GridSub->GridToWorldPosition(TargetGrid);
				CellCenter.Z += 10.0f;

				DrawDebugBox(
					World,
					CellCenter,
					CellExtent,
					OccupiedColor,
					false,
					0.03f,
					0,
					3.0f
				);
			}
		}
	}
#endif
}

void UKOGridBuildComponent::RequestBuild()
{
	if (CurrentMode != EKOGridBuildMode::Placing)
	{
		return;
	}
	
	UpdateGhostPreview();

	if (!bCurrentPlacementValid)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] 현재 위치에는 설치할 수 없습니다."));
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

	if (!GridSub->CanBuildArea(CurrentAnchor, CurrentBuildingSize, bCheckPlacementCollision))
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] 설치 불가 위치입니다. Grid: %d, %d"),
			CurrentAnchor.X,
			CurrentAnchor.Y
		);
		return;
	}
	
	FVector SpawnLocation = GridSub->GetAreaCenterWorldPosition(
		CurrentAnchor,
		CurrentBuildingSize
	);

	SpawnLocation.Z += CurrentFactoryRow->PlacementZOffset;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UClass* BuildingClass = CurrentBuildingClass.Get();
	if (!BuildingClass)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] BuildingClass 참조가 만료되었습니다."));
		return;
	}
	
	UKOInventoryComponent* InventoryComponent = GetInventoryComponent();
	if (!InventoryComponent)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] InventoryComponent를 찾을 수 없습니다."));
		return;
	}

	constexpr int32 BuildConsumeCount = 1;

	if (!InventoryComponent->HasEnoughItems(CurrentFactoryId, BuildConsumeCount))
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] 설비 아이템 수량이 부족합니다. FactoryId=%s"),
			*CurrentFactoryId.ToString()
		);
		return;
	}

	// 건물 스폰
	AKOBaseBuilding* NewBuilding = World->SpawnActor<AKOBaseBuilding>(
		BuildingClass,
		SpawnLocation,
		GetPlacementRotation(),
		SpawnParams
	);

	if (!NewBuilding)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] 건물 생성 실패"));
		return;
	}

	// 코너 벨트면 이웃에서 흐름 방향 자동 추론, 모호/이웃없음이면 수동(스크롤) flip 값으로 폴백.
	// (yaw 는 스폰 회전으로 이미 반영됨)
	if (AKOConveyorBelt* Belt = Cast<AKOConveyorBelt>(NewBuilding))
	{
		Belt->ApplyPlacementFlow(bCornerFlipPlacement);
	}

	// 스폰된 건물에 FactoryId 전달
	NewBuilding->InitializeBuildingData(CurrentFactoryId);

	// 그리드 점유처리
	GridSub->OccupyArea(
		CurrentAnchor,
		CurrentBuildingSize,
		NewBuilding
	);
	
	const FName BuiltFactoryId = CurrentFactoryId;

	if (!InventoryComponent->TryRemoveItem(BuiltFactoryId, BuildConsumeCount))
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Build] 설비 아이템 소모 실패. 설치를 롤백합니다. FactoryId=%s"),
			*BuiltFactoryId.ToString()
		);

		GridSub->FreeAreaByActor(NewBuilding);
		NewBuilding->Destroy();
		return;
	}

	UE_LOG(LogKOBuild, Log, TEXT("[Build] 건물 설치 완료: %s / Grid(%d, %d) / Size(%d, %d)"),
		*NewBuilding->GetName(),
		CurrentAnchor.X,
		CurrentAnchor.Y,
		CurrentBuildingSize.X,
		CurrentBuildingSize.Y
	);

	// 설치된 게 벨트면, 인접한 포트 보유 공장들과의 연결 팝업을 띄운다(없으면 무동작).
	// (CurrentAnchor/Size 가 아래 분기에서 리셋되기 전에 스냅샷 사용)
	if (AKOConveyorBelt* Belt = Cast<AKOConveyorBelt>(NewBuilding))
	{
		TryQueueBeltConnect(Belt, CurrentAnchor, CurrentBuildingSize);
	}

	const bool bFactoryDepleted = InventoryComponent->GetCountOf(BuiltFactoryId) <= 0;

	if (!bKeepBuildModeAfterPlacement || bFactoryDepleted)
	{
		CancelBuildMode();
	}
	else
	{
		UpdateGhostPreview();
	}
}

void UKOGridBuildComponent::OpenBeltConnectFor(AKOConveyorBelt* Belt)
{
	if (!Belt)
	{
		return;
	}

	UWorld* World = GetWorld();
	UKOGridSubsystem* GridSub = World ? World->GetSubsystem<UKOGridSubsystem>() : nullptr;
	if (!GridSub)
	{
		return;
	}

	// 이미 설치된 벨트라 그리드에서 점유 영역을 역조회해 배치 때와 동일 경로로 재사용.
	FIntPoint Anchor = FIntPoint::ZeroValue;
	FIntPoint Size   = FIntPoint(1, 1);
	if (!GridSub->TryGetOccupiedAreaForActor(Belt, Anchor, Size))
	{
		return;
	}

	TryQueueBeltConnect(Belt, Anchor, Size);
}

void UKOGridBuildComponent::TryQueueBeltConnect(AKOConveyorBelt* Belt, FIntPoint Anchor, FIntPoint Size)
{
	if (!Belt)
	{
		return;
	}

	UWorld* World = GetWorld();
	UKOGridSubsystem* GridSub = World ? World->GetSubsystem<UKOGridSubsystem>() : nullptr;
	if (!GridSub)
	{
		return;
	}

	// 결정적 스캔 순서(+X, -X, +Y, -Y). 다수 인접 공장은 이 순서대로 팝업 큐에 쌓인다.
	static const FIntPoint Dirs[4] = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };

	TArray<AKOBaseBuilding*> Factories;
	TSet<AActor*> Seen;

	for (int32 X = 0; X < Size.X; ++X)
	{
		for (int32 Y = 0; Y < Size.Y; ++Y)
		{
			const FIntPoint Cell = Anchor + FIntPoint(X, Y);
			for (const FIntPoint& Dir : Dirs)
			{
				const FIntPoint Neighbor = Cell + Dir;

				// 벨트 자신의 점유 영역 안쪽이면 스킵.
				const bool bInsideSelf =
					Neighbor.X >= Anchor.X && Neighbor.X < Anchor.X + Size.X &&
					Neighbor.Y >= Anchor.Y && Neighbor.Y < Anchor.Y + Size.Y;
				if (bInsideSelf)
				{
					continue;
				}

				AActor* Actor = GridSub->GetOccupyingActorAt(Neighbor);
				if (!Actor || Seen.Contains(Actor))
				{
					continue;
				}
				Seen.Add(Actor);

				// 공장이어야 하고, 벨트는 제외.
				AKOBaseBuilding* Building = Cast<AKOBaseBuilding>(Actor);
				if (!Building || Cast<AKOConveyorBelt>(Building))
				{
					continue;
				}

				// Processor 머신만 벨트 연결 팝업 대상. 레시피 미선택이어도 팝업은 띄움(빈 포트 표시).
				// Energy Producer 는 제외: 에너지 출력이라 포트 바인딩이 무의미하고, 연료 입력은
				// 기하 인접 시 tail-push 로 자동 공급되므로 명시적 바인딩 UI 가 불필요.
				const bool bIsProcessor =
					Building->FindComponentByClass<UKOFactoryProcessorComponent>() != nullptr;

				// 벨트 흐름축이 이 머신에 닿는 경우(설치 방향이 머신 입/출력과 맞는 경우)만 후보.
				// 수직 배치(흐름이 머신을 안 향함)는 연결 의미가 없어 제외.
				EKOPortKind ConnectKind;
				if (bIsProcessor && Belt->GetConnectablePortKind(Building, ConnectKind))
				{
					Factories.Add(Building);
				}
			}
		}
	}

	if (Factories.Num() == 0)
	{
		return;
	}

	PendingConnectBelt = Belt;
	PendingConnectFactories.Reset();
	for (AKOBaseBuilding* Factory : Factories)
	{
		PendingConnectFactories.Add(Factory);
	}
	PendingConnectIndex = 0;

	OpenNextBeltConnectPopup();
}

void UKOGridBuildComponent::OpenNextBeltConnectPopup()
{
	AKOConveyorBelt* Belt = PendingConnectBelt.Get();
	if (!Belt)
	{
		PendingConnectFactories.Reset();
		PendingConnectIndex = 0;
		return;
	}

	// 큐에서 아직 살아있는 다음 공장을 찾는다.
	AKOBaseBuilding* Factory = nullptr;
	while (PendingConnectIndex < PendingConnectFactories.Num())
	{
		Factory = PendingConnectFactories[PendingConnectIndex++].Get();
		if (Factory)
		{
			break;
		}
		Factory = nullptr;
	}

	if (!Factory)
	{
		// 큐 소진.
		PendingConnectFactories.Reset();
		PendingConnectIndex = 0;
		return;
	}

	UCommonActivatableWidget* Widget = UKOUISubsystem::OpenWidget(this, KOGameplayTags::UI_Widget_BeltConnect);
	UKOBeltConnectWidget* ConnectWidget = Cast<UKOBeltConnectWidget>(Widget);
	if (!ConnectWidget)
	{
		// 오픈 실패(미등록 등) — 큐 중단.
		PendingConnectFactories.Reset();
		PendingConnectIndex = 0;
		return;
	}

	ConnectWidget->SetupConnection(Belt, Factory);

	// 닫히면(슬롯 선택 or ESC) 다음 공장 팝업으로 진행.
	Widget->OnDeactivated().AddWeakLambda(this, [this]()
	{
		OpenNextBeltConnectPopup();
	});
}

void UKOGridBuildComponent::CancelCurrentMode()
{
	switch (CurrentMode)
	{
	case EKOGridBuildMode::Placing:
		CancelBuildMode();
		break;

	case EKOGridBuildMode::Destroying:
		CancelDestroyMode();
		break;

	case EKOGridBuildMode::BuildMenu:
		ExitBuildMenuMode();
		break;

	case EKOGridBuildMode::None:
	default:
		break;
	}
}

void UKOGridBuildComponent::CancelBuildMode()
{
	if (CurrentMode != EKOGridBuildMode::Placing)
	{
		return;
	}
	
	ClearPlacementState();

	SetCurrentMode(EKOGridBuildMode::BuildMenu);
	SetComponentTickEnabled(false);

	UE_LOG(LogKOBuild, Log, TEXT("[Build] 설치 모드 종료 - 건설 메뉴로 복귀"));
}

void UKOGridBuildComponent::StartDestroyMode()
{
	if (!IsBuildSystemActive())
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Destroy] 건설 메뉴 모드가 아닐 때는 파괴 모드로 들어갈 수 없습니다."));
		return;
	}

	if (CurrentMode == EKOGridBuildMode::Destroying)
	{
		return;
	}
	
	CancelCurrentMode();

	SetCurrentMode(EKOGridBuildMode::Destroying);
	SetComponentTickEnabled(true);

	UpdateDestroyTargetPreview();

	UE_LOG(LogKOBuild, Log, TEXT("[Destroy] 건물 파괴 모드 시작"));
}

void UKOGridBuildComponent::CancelDestroyMode()
{
	if (CurrentMode != EKOGridBuildMode::Destroying)
	{
		return;
	}
	
	ClearDestroyTargetActor();

	SetCurrentMode(EKOGridBuildMode::BuildMenu);
	SetComponentTickEnabled(false);

	UE_LOG(LogKOBuild, Log, TEXT("[Destroy] 건물 파괴 모드 종료 - 건설 메뉴로 복귀"));
}

void UKOGridBuildComponent::RotatePlacementPreview(int32 Direction)
{
	if (CurrentMode != EKOGridBuildMode::Placing)
	{
		return;
	}

	if (Direction == 0)
	{
		return;
	}

	const int32 Step = Direction > 0 ? 1 : -1;

	const int32 OldRotationStep = CurrentRotationStep;
	CurrentRotationStep = (CurrentRotationStep + Step + 4) % 4;

	// 코너 벨트: yaw 가 4단계 경계(3↔0)를 넘을 때마다 흐름 반전 토글 → 4 yaw × 2 flip = 8방향 순환.
	if (IsCurrentBuildingCornerBelt())
	{
		const bool bCrossedForward  = (Step > 0 && OldRotationStep == 3);
		const bool bCrossedBackward = (Step < 0 && OldRotationStep == 0);
		if (bCrossedForward || bCrossedBackward)
		{
			bCornerFlipPlacement = !bCornerFlipPlacement;
		}
	}

	// 2x1 같은 건물은 90도 회전하면 1x2가 되어야 함
	CurrentBuildingSize = GetRotatedBuildingSize();

	if (CurrentPreviewActor)
	{
		CurrentPreviewActor->SetActorRotation(GetPlacementRotation());
	}

	UpdateGhostPreview();
}

void UKOGridBuildComponent::RequestDestroy()
{
	if (CurrentMode != EKOGridBuildMode::Destroying)
	{
		return;
	}
	
	UpdateDestroyTargetPreview();

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

	AActor* TargetActor = CurrentDestroyTargetActor.Get();

	if (!IsValid(TargetActor))
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Destroy] 파괴할 대상이 없습니다."));
		return;
	}
	
	AKOBaseBuilding* TargetBuilding = Cast<AKOBaseBuilding>(TargetActor);
	if (!TargetBuilding)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Destroy] 대상이 AKOBaseBuilding이 아닙니다: %s"),
			*TargetActor->GetName()
		);
		return;
	}

	const FName FactoryId = TargetBuilding->GetFactoryId();
	if (FactoryId.IsNone())
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Destroy] 파괴 대상의 FactoryId가 비어 있습니다: %s"),
			*TargetBuilding->GetName()
		);
		return;
	}

	UKOInventoryComponent* InventoryComponent = GetInventoryComponent();
	if (!InventoryComponent)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Destroy] InventoryComponent를 찾을 수 없습니다."));
		return;
	}
	
	const int32 Remaining = InventoryComponent->TryAddItem(
		EKOSlotKind::Factory,
		FactoryId,
		1
	);

	if (Remaining > 0)
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Destroy] 인벤토리에 공간이 없어 파괴를 취소합니다. FactoryId=%s"),
			*FactoryId.ToString()
		);
		return;
	}
	
	if (!GridSub->FreeAreaByActor(TargetActor))
	{
		UE_LOG(LogKOBuild, Warning, TEXT("[Destroy] 점유 해제 실패: %s"),
			*TargetActor->GetName()
		);

		InventoryComponent->TryRemoveItem(FactoryId, 1);
		return;
	}

	UE_LOG(LogKOBuild, Log, TEXT("[Destroy] 건물 파괴 완료: %s"),
		*TargetBuilding->GetName()
	);
	
	ClearDestroyTargetActor();

	TargetBuilding->Destroy();
}

void UKOGridBuildComponent::SetPreviewActorBuildableState(bool bCanBuild)
{
	if (!CurrentPreviewActor)
	{
		return;
	}
	
	if (bHasLastPreviewBuildableState && bLastPreviewBuildableState == bCanBuild)
	{
		return;
	}

	UMaterialInterface* TargetMaterial = bCanBuild
		? BuildableGhostMaterial
		: NotBuildableGhostMaterial;

	ApplyGhostMaterial(CurrentPreviewActor, TargetMaterial);
	
	bHasLastPreviewBuildableState = true;
	bLastPreviewBuildableState = bCanBuild;
}

void UKOGridBuildComponent::ApplyGhostMaterial(AActor* TargetActor, UMaterialInterface* TargetMaterial) const
{
	if (!TargetActor || !TargetMaterial)
	{
		return;
	}

	TArray<UMeshComponent*> MeshComponents;
	TargetActor->GetComponents<UMeshComponent>(MeshComponents);

	// 머티리얼이 여러개 있을 수 있음
	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent)
		{
			continue;
		}

		const int32 MaterialCount = MeshComponent->GetNumMaterials();

		for (int32 Index = 0; Index < MaterialCount; ++Index)
		{
			MeshComponent->SetMaterial(Index, TargetMaterial);
		}
	}
}

void UKOGridBuildComponent::UpdateDestroyTargetPreview()
{
	if (CurrentMode != EKOGridBuildMode::Destroying)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		ClearDestroyTargetActor();
		return;
	}

	UKOGridSubsystem* GridSub = World->GetSubsystem<UKOGridSubsystem>();
	if (!GridSub)
	{
		ClearDestroyTargetActor();
		return;
	}

	FHitResult HitResult;

	if (!TraceFromScreenCenter(HitResult, DestroyTraceChannel))
	{
		ClearDestroyTargetActor();
		return;
	}

	AActor* HitActor = HitResult.GetActor();

	if (!IsValid(HitActor))
	{
		ClearDestroyTargetActor();
		return;
	}

	FIntPoint OccupiedAnchor;
	FIntPoint OccupiedSize;

	// 라인트레이스에 맞은 Actor가 그리드에 등록된 건물인지 확인
	if (!GridSub->TryGetOccupiedAreaForActor(
		HitActor,
		OccupiedAnchor,
		OccupiedSize
	))
	{
		ClearDestroyTargetActor();
		return;
	}

	SetDestroyTargetActor(HitActor);
}

void UKOGridBuildComponent::SetDestroyTargetActor(AActor* NewTargetActor)
{
	if (CurrentDestroyTargetActor.Get() == NewTargetActor)
	{
		return;
	}

	ClearDestroyTargetActor();

	if (!NewTargetActor)
	{
		return;
	}

	CurrentDestroyTargetActor = NewTargetActor;
	ApplyDestroyTargetMaterial(NewTargetActor);
}

void UKOGridBuildComponent::ClearDestroyTargetActor()
{
	RestoreDestroyTargetMaterial();

	CurrentDestroyTargetActor = nullptr;
	DestroyTargetOriginalMaterials.Empty();
}

void UKOGridBuildComponent::ApplyDestroyTargetMaterial(AActor* TargetActor)
{
	if (!TargetActor || !DestroyTargetMaterial)
	{
		return;
	}

	TArray<UMeshComponent*> MeshComponents;
	TargetActor->GetComponents<UMeshComponent>(MeshComponents);

	for (UMeshComponent* MeshComponent : MeshComponents)
	{
		if (!MeshComponent)
		{
			continue;
		}

		FKODestroyTargetOriginalMaterials OriginalData;
		OriginalData.MeshComponent = MeshComponent;

		const int32 MaterialCount = MeshComponent->GetNumMaterials();

		for (int32 Index = 0; Index < MaterialCount; ++Index)
		{
			OriginalData.Materials.Add(MeshComponent->GetMaterial(Index));
			MeshComponent->SetMaterial(Index, DestroyTargetMaterial);
		}

		DestroyTargetOriginalMaterials.Add(OriginalData);
	}
}

void UKOGridBuildComponent::RestoreDestroyTargetMaterial()
{
	for (const FKODestroyTargetOriginalMaterials& OriginalData : DestroyTargetOriginalMaterials)
	{
		UMeshComponent* MeshComponent = OriginalData.MeshComponent.Get();

		if (!IsValid(MeshComponent))
		{
			continue;
		}

		for (int32 Index = 0; Index < OriginalData.Materials.Num(); ++Index)
		{
			MeshComponent->SetMaterial(
				Index,
				OriginalData.Materials[Index]
			);
		}
	}
}

UKOInventoryComponent* UKOGridBuildComponent::GetInventoryComponent() const
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		return nullptr;
	}

	if (UKOInventoryComponent* InventoryComponent = PC->FindComponentByClass<UKOInventoryComponent>())
	{
		return InventoryComponent;
	}

	if (APawn* Pawn = PC->GetPawn())
	{
		return Pawn->FindComponentByClass<UKOInventoryComponent>();
	}

	return nullptr;
}

void UKOGridBuildComponent::SetCurrentMode(EKOGridBuildMode NewMode)
{
	if (CurrentMode == NewMode)
	{
		return;
	}

	const EKOGridBuildMode PreviousMode = CurrentMode;
	CurrentMode = NewMode;

	FKOBuildModeChangedMessage Message;
	Message.PreviousMode = PreviousMode;
	Message.NewMode = NewMode;

	Broadcast(
		KOGameplayTags::Data_Message_Build_ModeChanged,
		FInstancedStruct::Make(Message)
	);

	UE_LOG(
		LogKOBuild,
		Log,
		TEXT("[Build] Mode Changed: %d -> %d"),
		static_cast<uint8>(PreviousMode),
		static_cast<uint8>(NewMode)
	);
}

FRotator UKOGridBuildComponent::GetPlacementRotation() const
{
	return FRotator(0.0f, CurrentRotationStep * 90.0f, 0.0f);
}

FIntPoint UKOGridBuildComponent::GetRotatedBuildingSize() const
{
	// 0도, 180도는 원래 크기
	if (CurrentRotationStep % 2 == 0)
	{
		return BaseBuildingSize;
	}

	// 90도, 270도는 X/Y 교환
	return FIntPoint(BaseBuildingSize.Y, BaseBuildingSize.X);
}

bool UKOGridBuildComponent::IsCurrentBuildingCornerBelt() const
{
	UClass* BuildingClass = CurrentBuildingClass.Get();
	if (!BuildingClass || !BuildingClass->IsChildOf(AKOConveyorBelt::StaticClass()))
	{
		return false;
	}

	const AKOConveyorBelt* BeltCDO = Cast<AKOConveyorBelt>(BuildingClass->GetDefaultObject());
	return BeltCDO && BeltCDO->GetShape() == EKOBeltShape::Corner;
}

bool UKOGridBuildComponent::TraceFromScreenCenter(FHitResult& OutHit, ECollisionChannel TraceChannel) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC)
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * TraceDistance;

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;

	if (APawn* Pawn = PC->GetPawn())
	{
		QueryParams.AddIgnoredActor(Pawn);
	}

	if (CurrentPreviewActor)
	{
		QueryParams.AddIgnoredActor(CurrentPreviewActor);
	}

	const bool bHit = World->LineTraceSingleByChannel(
		OutHit,
		TraceStart,
		TraceEnd,
		TraceChannel,
		QueryParams
	);
	
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (
		IsInGameThread() &&
		CVarKODrawBuildTrace.GetValueOnGameThread() != 0
	)
	{
		const float DebugLifeTime = 0.03f;
		const float DebugThickness = 2.0f;

		if (bHit)
		{
			// 카메라에서 맞은 지점까지 초록색 라인
			DrawDebugLine(
				World,
				TraceStart,
				OutHit.ImpactPoint,
				FColor::Green,
				false,
				DebugLifeTime,
				0,
				DebugThickness
			);

			// 맞은 지점 표시
			DrawDebugSphere(
				World,
				OutHit.ImpactPoint,
				12.0f,
				12,
				FColor::Green,
				false,
				DebugLifeTime,
				0,
				1.5f
			);
		}
		else
		{
			// 아무것도 맞지 않으면 전체 라인 빨간색
			DrawDebugLine(
				World,
				TraceStart,
				TraceEnd,
				FColor::Red,
				false,
				DebugLifeTime,
				0,
				DebugThickness
			);
		}
	}
#endif

	return bHit;	
}
