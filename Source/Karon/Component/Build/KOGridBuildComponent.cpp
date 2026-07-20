#include "Component/Build/KOGridBuildComponent.h"
#include "Data/KODataTableTypes.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "SubSystem/KOGridSubSystem.h"
#include "Building/KOBaseBuilding.h"
#include "Building/KOGhostPreview.h"
#include "Building/Conveyor/KOConveyorBelt.h"
#include "Building/KOGridVisual.h"
#include "Building/Conveyor/KOConveyorFlowResolver.h"
#include "EngineUtils.h"
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
#include "Component/Factory/KOEnergyProducerComponent.h"
#include "CommonActivatableWidget.h"
#include "Subsystem/KOQuestGuideSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundConcurrency.h"

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
	
	const bool bIsConveyorPreview =
		CurrentBuildingClass.IsValid() &&
		CurrentBuildingClass->IsChildOf(AKOConveyorBelt::StaticClass());
	
	// 컨베이어 벨트면 고스트 프리뷰에 진행 방향 화살표를 표시한다.
	if (bIsConveyorPreview)
	{
		const float ArrowYaw = GetPreviewConveyorArrowYaw();
		CurrentPreviewActor->ShowDirectionArrow(ArrowYaw);
	}
	else
	{
		CurrentPreviewActor->HideDirectionArrow();
	}

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
	GridSub->OccupyArea(CurrentAnchor, CurrentBuildingSize, NewBuilding);
	
	// 퀘스트
	if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
	{
		QuestGuide->NotifyBuildingPlaced(CurrentFactoryId);
	}
	
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
	
	if (AKOGridVisual* GridVisual = FindGridVisualActor())
	{
		GridVisual->RefreshInstalledPowerCoverage();
	}

	UE_LOG(LogKOBuild, Log, TEXT("[Build] 건물 설치 완료: %s / Grid(%d, %d) / Size(%d, %d)"),
		*NewBuilding->GetName(),
		CurrentAnchor.X,
		CurrentAnchor.Y,
		CurrentBuildingSize.X,
		CurrentBuildingSize.Y
	);
	
	// 사운드
	const bool bIsConveyor = Cast<AKOConveyorBelt>(NewBuilding) != nullptr;
	const FKOBuildSoundSettings& InstallSoundSettings = bIsConveyor ? ConveyorInstallSound : FactoryInstallSound;
	// StartTime으로 건너뛴 부분을 제외한 실제 남은 재생 시간
	float InstallSoundDuration = 0.0f;

	if (InstallSoundSettings.Sound)
	{
		InstallSoundDuration = FMath::Max(
			0.0f,
			InstallSoundSettings.Sound->GetDuration()
				- InstallSoundSettings.StartTime
		);
	}

	// 설치음보다 가동음이 먼저 또는 동시에 나오지 않도록 차단
	NewBuilding->BlockOperatingSound(InstallSoundDuration);
	PlayBuildSound(InstallSoundSettings, NewBuilding->GetActorLocation());

	// 설치된 게 벨트면, 인접한 포트 보유 공장들과의 연결 팝업을 띄운다(없으면 무동작).
	// (CurrentAnchor/Size 가 아래 분기에서 리셋되기 전에 스냅샷 사용)
	if (AKOConveyorBelt* Belt = Cast<AKOConveyorBelt>(NewBuilding))
	{
		TryQueueBeltConnect(Belt);
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

	TryQueueBeltConnect(Belt);
}

void UKOGridBuildComponent::TryQueueBeltConnect(AKOConveyorBelt* Belt)
{
	TArray<AKOBaseBuilding*> Factories;

	if (!FindConnectableOutputFactoriesForBelt(Belt, Factories))
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

void UKOGridBuildComponent::UpdateGridVisualVisibility()
{
	AKOGridVisual* GridVisualActor = FindGridVisualActor();
	if (!GridVisualActor)
	{
		return;
	}

	const bool bShouldShowGrid =
		CurrentMode == EKOGridBuildMode::BuildMenu ||
		CurrentMode == EKOGridBuildMode::Placing ||
		CurrentMode == EKOGridBuildMode::Destroying;

	GridVisualActor->SetGridVisible(bShouldShowGrid);
	
	if (bShouldShowGrid)
	{
		GridVisualActor->RefreshInstalledPowerCoverage();
	}
	else
	{
		GridVisualActor->ClearPowerCoverageCells();
	}
}

AKOGridVisual* UKOGridBuildComponent::FindGridVisualActor()
{
	if (CachedGridVisualActor)
	{
		return CachedGridVisualActor;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	for (TActorIterator<AKOGridVisual> It(World); It; ++It)
	{
		CachedGridVisualActor = *It;
		return CachedGridVisualActor;
	}

	return nullptr;
}

void UKOGridBuildComponent::PlayBuildSound(const FKOBuildSoundSettings& SoundSettings, const FVector& Location) const
{
	if (!SoundSettings.Sound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		SoundSettings.Sound,
		Location,
		FRotator::ZeroRotator,
		SoundSettings.Volume,
		1.0f,			// Pitch
		SoundSettings.StartTime
	);
}

bool UKOGridBuildComponent::FindConnectableOutputFactoriesForBelt(AKOConveyorBelt* Belt,
                                                                  TArray<AKOBaseBuilding*>& OutFactories) const
{
	OutFactories.Reset();

	if (!Belt)
	{
		return false;
	}

	UWorld* World = GetWorld();
	UKOGridSubsystem* GridSub = World ? World->GetSubsystem<UKOGridSubsystem>() : nullptr;
	if (!GridSub)
	{
		return false;
	}

	FIntPoint Anchor = FIntPoint::ZeroValue;
	FIntPoint Size = FIntPoint(1, 1);

	if (!GridSub->TryGetOccupiedAreaForActor(Belt, Anchor, Size))
	{
		return false;
	}

	static const FIntPoint Dirs[4] =
	{
		FIntPoint(1, 0),
		FIntPoint(-1, 0),
		FIntPoint(0, 1),
		FIntPoint(0, -1)
	};

	TSet<AActor*> Seen;

	for (int32 X = 0; X < Size.X; ++X)
	{
		for (int32 Y = 0; Y < Size.Y; ++Y)
		{
			const FIntPoint Cell = Anchor + FIntPoint(X, Y);

			for (const FIntPoint& Dir : Dirs)
			{
				const FIntPoint Neighbor = Cell + Dir;

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

				AKOBaseBuilding* Building = Cast<AKOBaseBuilding>(Actor);
				if (!Building || Cast<AKOConveyorBelt>(Building))
				{
					continue;
				}

				const bool bIsProcessor =
					Building->FindComponentByClass<UKOFactoryProcessorComponent>() != nullptr;

				EKOPortKind ConnectKind;
				if (bIsProcessor && Belt->GetConnectablePortKind(Building, ConnectKind))
				{
					if (ConnectKind == EKOPortKind::Output)
					{
						OutFactories.Add(Building);
					}
				}
			}
		}
	}

	return OutFactories.Num() > 0;
}

float UKOGridBuildComponent::GetPreviewConveyorArrowYaw() const
{
	UClass* BuildingClass = CurrentBuildingClass.Get();

	if (!BuildingClass || !BuildingClass->IsChildOf(AKOConveyorBelt::StaticClass()))
	{
		return 0.0f;
	}

	const AKOConveyorBelt* BeltCDO = Cast<AKOConveyorBelt>(BuildingClass->GetDefaultObject());

	if (!BeltCDO)
	{
		return 0.0f;
	}

	const UWorld* World = GetWorld();
	if (!World)
	{
		return 0.0f;
	}

	const FIntPoint MyCell = CurrentAnchor;
	const FIntPoint Forward = GetPreviewForwardStep();
	const FIntPoint Side = GetPreviewSideStep();

	if (BeltCDO->GetShape() == EKOBeltShape::Corner)
	{
		bool bResolvedFlip = false;

		const bool bResolved =
			FKOConveyorFlowResolver::TryResolveCornerFlip(
				World,
				MyCell,
				Forward,
				Side,
				bResolvedFlip
			);

		const bool bFinalFlip = bResolved ? bResolvedFlip : bCornerFlipPlacement;
		
		return bFinalFlip ? 90.0f : 0.0f;
	}

	bool bResolvedReverse = false;

	const bool bResolved =
		FKOConveyorFlowResolver::TryResolveStraightReverse(
			World,
			MyCell,
			Forward,
			bResolvedReverse
		);
	
	const bool bFinalReverse = bResolved ? bResolvedReverse : false;

	return bFinalReverse ? 180.0f : 0.0f;
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
	
	for (TActorIterator<AKOBaseBuilding> It(GetWorld()); It; ++It)
	{
		if (AKOBaseBuilding* Building = *It)
		{
			Building->SetPressureWarningSuppressed(true);
		}
	}

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
	
	for (TActorIterator<AKOBaseBuilding> It(GetWorld()); It; ++It)
	{
		if (AKOBaseBuilding* Building = *It)
		{
			Building->SetPressureWarningSuppressed(false);
		}
	}

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

bool UKOGridBuildComponent::CanOpenBeltConnectFor(AKOConveyorBelt* Belt) const
{
	TArray<AKOBaseBuilding*> Factories;
	return FindConnectableOutputFactoriesForBelt(Belt, Factories);
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

	// 설비/벨트 내부에 적재돼 있던 아이템도 인벤토리로 환급(설치 아이템 자체는 위에서 이미 환급).
	// 인벤토리가 가득 차 일부가 들어가지 못하면 그만큼은 소실되며 경고만 남긴다(파괴는 그대로 진행).
	RefundStoredItems(TargetBuilding, *InventoryComponent);

	UE_LOG(LogKOBuild, Log, TEXT("[Destroy] 건물 파괴 완료: %s"),
		*TargetBuilding->GetName()
	);
	
	
	if (AKOGridVisual* GridVisual = FindGridVisualActor())
	{
		GridVisual->RefreshInstalledPowerCoverage();
	}

	// 사운드
	const FVector DestroySoundLocation = TargetBuilding->GetActorLocation();
	const bool bIsConveyor = Cast<AKOConveyorBelt>(TargetBuilding) != nullptr;
	const FKOBuildSoundSettings& DestroySoundSettings = bIsConveyor ? ConveyorDestroySound : FactoryDestroySound;
	TargetBuilding->StopOperatingSoundImmediately(); // 해제음이 나오기 전에 가동음을 Fade 없이 즉시 정지
	
	ClearDestroyTargetActor();

	if (TargetBuilding->Destroy())
	{
		PlayBuildSound(DestroySoundSettings, DestroySoundLocation);
	}
}

void UKOGridBuildComponent::RefundStoredItems(AKOBaseBuilding* TargetBuilding, UKOInventoryComponent& InventoryComponent) const
{
	if (!TargetBuilding)
	{
		return;
	}

	// 내부 적재 아이템을 (ItemId → 수량)으로 합산.
	TMap<FName, int32> StoredItems;

	// 벨트: 벨트 위를 흐르던 화물.
	if (const AKOConveyorBelt* Belt = Cast<AKOConveyorBelt>(TargetBuilding))
	{
		Belt->CollectCargoItems(StoredItems);
	}

	// 가공 설비: 입력/출력 버퍼.
	if (const UKOFactoryProcessorComponent* Processor = TargetBuilding->FindComponentByClass<UKOFactoryProcessorComponent>())
	{
		for (const TPair<FName, int32>& Pair : Processor->GetInputBuffer())
		{
			if (!Pair.Key.IsNone() && Pair.Value > 0)
			{
				StoredItems.FindOrAdd(Pair.Key) += Pair.Value;
			}
		}
		for (const TPair<FName, int32>& Pair : Processor->GetOutputBuffer())
		{
			if (!Pair.Key.IsNone() && Pair.Value > 0)
			{
				StoredItems.FindOrAdd(Pair.Key) += Pair.Value;
			}
		}
	}

	// 생산 설비: 연료 버퍼.
	if (const UKOEnergyProducerComponent* Producer = TargetBuilding->FindComponentByClass<UKOEnergyProducerComponent>())
	{
		const FName FuelItemId = Producer->GetFuelItemId();
		const int32 FuelCount  = Producer->GetFuelCount();
		if (!FuelItemId.IsNone() && FuelCount > 0)
		{
			StoredItems.FindOrAdd(FuelItemId) += FuelCount;
		}
	}

	// 합산된 적재 아이템을 인벤토리로 환급(들어가지 못한 잔량은 소실 — 경고).
	for (const TPair<FName, int32>& Pair : StoredItems)
	{
		const int32 Remaining = InventoryComponent.TryAddItem(EKOSlotKind::Item, Pair.Key, Pair.Value);
		if (Remaining > 0)
		{
			UE_LOG(LogKOBuild, Warning, TEXT("[Destroy] 인벤토리 공간 부족으로 적재 아이템 일부 소실: ItemId=%s, 소실=%d"),
				*Pair.Key.ToString(),
				Remaining
			);
		}
	}
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
	
	UpdateGridVisualVisibility();

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

FIntPoint UKOGridBuildComponent::GetPreviewForwardStep() const
{
	switch ((CurrentRotationStep % 4 + 4) % 4)
	{
	case 0:
		return FIntPoint(1, 0);

	case 1:
		return FIntPoint(0, 1);

	case 2:
		return FIntPoint(-1, 0);

	case 3:
		return FIntPoint(0, -1);

	default:
		return FIntPoint(1, 0);
	}
}

FIntPoint UKOGridBuildComponent::GetPreviewSideStep() const
{
	const FIntPoint Forward = GetPreviewForwardStep();

	// 로컬 +Y 방향.
	// Forward=(1,0)이면 Side=(0,1)
	return FIntPoint(-Forward.Y, Forward.X);
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

	return bHit;	
}
