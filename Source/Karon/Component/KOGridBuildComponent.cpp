#include "KOGridBuildComponent.h"
#include "Data/KOBuildingDataAsset.h"
#include "SubSystem/KOGridSubsystem.h"
#include "DrawDebugHelpers.h"

#include "Components/MeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Building/KOGhostPreview.h"

UKOGridBuildComponent::UKOGridBuildComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// 건설 모드가 켜졌을 때만 tick 켬
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

	if (!bIsBuildMode)
	{
		return;
	}

	UpdateGhostPreview(); // 고스트 프리뷰 위치 업데이트
}

void UKOGridBuildComponent::StartAssignedBuildMode()
{
	if (!DefaultBuildingData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] DefaultBuildingData가 할당되지 않았습니다."));
		return;
	}

	StartBuildModeWithData(DefaultBuildingData);
}

void UKOGridBuildComponent::StartBuildModeWithData(UKOBuildingDataAsset* BuildingData)
{
	if (!BuildingData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] BuildingData가 없습니다."));
		return;
	}

	if (!BuildingData->BuildingClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] BuildingClass가 없습니다."));
		return;
	}
	
	if (!PreviewActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] PreviewActorClass가 설정되지 않았습니다."));
		return;
	}

	if (BuildingData->GridSize.X <= 0 || BuildingData->GridSize.Y <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] GridSize가 잘못되었습니다."));
		return;
	}

	DestroyPreviewActor();

	CurrentBuildingData = BuildingData;
	CurrentBuildingSize = BuildingData->GridSize;

	bIsBuildMode = true;
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

	if (!CurrentBuildingData || !CurrentBuildingData->BuildingClass)
	{
		return false;
	}
	
	if (!PreviewActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] PreviewActorClass가 설정되지 않았습니다."));
		return false;
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
		UE_LOG(LogTemp, Warning, TEXT("[Build] PreviewActor 생성 실패"));
		return false;
	}
	
	// 건물 BP의 StaticMesh를 읽어서 고스트 Actor에 복사
	CurrentPreviewActor = PreviewActor;
	PreviewActor->SetupFromBuildingClass(CurrentBuildingData->BuildingClass);
	
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

void UKOGridBuildComponent::UpdateGhostPreview()
{
	if (!CurrentBuildingData)
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

	if (!TraceFromScreenCenter(HitResult))
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
	PreviewLocation.Z += CurrentBuildingData->PlacementZOffset;
	CurrentPreviewActor->SetActorLocation(PreviewLocation);

	// 설치할 수 있는지 검사
	const bool bCanBuild = GridSub->CanBuildArea(
		CurrentAnchor,
		CurrentBuildingSize,
		bCheckPlacementCollision
	);

	bCurrentPlacementValid = bCanBuild;

	SetPreviewActorBuildableState(bCanBuild);
	
	// 건물이 차지할 그리드 셀 디버그 표시
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (IsInGameThread())
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
	if (!bIsBuildMode)
	{
		return;
	}
	
	UpdateGhostPreview();

	if (!bCurrentPlacementValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] 현재 위치에는 설치할 수 없습니다."));
		return;
	}

	if (!CurrentPreviewActor || CurrentPreviewActor->IsHidden())
	{
		return;
	}

	if (!CurrentBuildingData || !CurrentBuildingData->BuildingClass)
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

	if (!GridSub->CanBuildArea(CurrentAnchor, CurrentBuildingSize, bCheckPlacementCollision))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] 설치 불가 위치입니다. Grid: %d, %d"),
			CurrentAnchor.X,
			CurrentAnchor.Y
		);
		return;
	}
	
	FVector SpawnLocation = GridSub->GetAreaCenterWorldPosition(
		CurrentAnchor,
		CurrentBuildingSize
	);

	SpawnLocation.Z += CurrentBuildingData->PlacementZOffset;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 건물 스폰
	AActor* NewBuilding = World->SpawnActor<AActor>(
		CurrentBuildingData->BuildingClass,
		SpawnLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (!NewBuilding)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] 건물 생성 실패"));
		return;
	}

	// 그리드 점유처리
	GridSub->OccupyArea(
		CurrentAnchor,
		CurrentBuildingSize,
		NewBuilding
	);

	UE_LOG(LogTemp, Log, TEXT("[Build] 건물 설치 완료: %s / Grid(%d, %d) / Size(%d, %d)"),
		*NewBuilding->GetName(),
		CurrentAnchor.X,
		CurrentAnchor.Y,
		CurrentBuildingSize.X,
		CurrentBuildingSize.Y
	);

	if (!bKeepBuildModeAfterPlacement)
	{
		CancelBuildMode();
	}
	else
	{
		UpdateGhostPreview();
	}
}

void UKOGridBuildComponent::CancelBuildMode()
{
	bIsBuildMode = false;
	SetComponentTickEnabled(false);
	bCurrentPlacementValid = false;

	DestroyPreviewActor();

	CurrentBuildingData = nullptr;
	CurrentAnchor = FIntPoint::ZeroValue;
	CurrentBuildingSize = FIntPoint(1, 1);
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

bool UKOGridBuildComponent::TraceFromScreenCenter(FHitResult& OutHit) const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	APlayerController* PC = GetOwningPlayerController();
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

	if (AActor* OwnerActor = GetOwner())
	{
		QueryParams.AddIgnoredActor(OwnerActor);
	}

	if (CurrentPreviewActor)
	{
		QueryParams.AddIgnoredActor(CurrentPreviewActor);
	}

	const bool bHit = World->LineTraceSingleByChannel(
		OutHit,
		TraceStart,
		TraceEnd,
		GroundTraceChannel,
		QueryParams
	);

	return bHit;	
}

APlayerController* UKOGridBuildComponent::GetOwningPlayerController() const
{
	AActor* OwnerActor = GetOwner();

	if (OwnerActor)
	{
		if (APlayerController* PC = Cast<APlayerController>(OwnerActor))
		{
			return PC;
		}

		if (APawn* Pawn = Cast<APawn>(OwnerActor))
		{
			return Cast<APlayerController>(Pawn->GetController());
		}
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	return UGameplayStatics::GetPlayerController(World, 0);
}