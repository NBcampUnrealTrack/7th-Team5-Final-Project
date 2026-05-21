#include "KOGridBuildComponent.h"
#include "Data/KOBuildingDataAsset.h"
#include "SubSystem/KOGridSubsystem.h"
#include "Building/KOBaseBuilding.h"
#include "DrawDebugHelpers.h"

#include "Components/MeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInterface.h"
#include "Building/KOGhostPreview.h"

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

	if (bIsBuildMode)
	{
		UpdateGhostPreview();
		return;
	}

	if (bIsDestroyMode)
	{
		UpdateDestroyTargetPreview();
	}
}

void UKOGridBuildComponent::StartBuildModeByIndex(int32 BuildIndex)
{
	if (!BuildOptions.IsValidIndex(BuildIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] 잘못된 건물 인덱스입니다. Index: %d"), BuildIndex);
		return;
	}

	UKOBuildingDataAsset* SelectedBuildingData = BuildOptions[BuildIndex];

	if (!SelectedBuildingData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Build] 선택된 BuildingData가 없습니다. Index: %d"), BuildIndex);
		return;
	}

	StartBuildModeWithData(SelectedBuildingData);
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

	CancelDestroyMode();
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
	PreviewActor->SetupFromBuildingClass(CurrentBuildingData->BuildingClass.Get());
	
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
	AKOBaseBuilding* NewBuilding = World->SpawnActor<AKOBaseBuilding>(
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
	
	// 스폰된 건물에 DataAsset 전달
	NewBuilding->InitializeBuildingData(CurrentBuildingData);

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
	bCurrentPlacementValid = false;

	DestroyPreviewActor();

	CurrentBuildingData = nullptr;
	CurrentAnchor = FIntPoint::ZeroValue;
	CurrentBuildingSize = FIntPoint(1, 1);
	
	if (!bIsDestroyMode)
	{
		SetComponentTickEnabled(false);
	}
}

void UKOGridBuildComponent::StartDestroyMode()
{
	if (bIsBuildMode)
	{
		CancelBuildMode();
	}

	bIsDestroyMode = true;
	SetComponentTickEnabled(true);

	UpdateDestroyTargetPreview();

	UE_LOG(LogTemp, Log, TEXT("[Destroy] 건물 파괴 모드 시작"));
}

void UKOGridBuildComponent::CancelDestroyMode()
{
	if (!bIsDestroyMode)
	{
		return;
	}
	
	bIsDestroyMode = false;
	
	ClearDestroyTargetActor();

	if (!bIsBuildMode)
	{
		SetComponentTickEnabled(false);
	}

	UE_LOG(LogTemp, Log, TEXT("[Destroy] 건물 파괴 모드 종료"));
}

void UKOGridBuildComponent::RequestDestroy()
{
	if (!bIsDestroyMode)
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

	AActor* TargetBuilding = CurrentDestroyTargetActor.Get();

	if (!IsValid(TargetBuilding))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Destroy] 파괴할 대상이 없습니다."));
		return;
	}

	if (!GridSub->FreeAreaByActor(TargetBuilding))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Destroy] 점유 해제 실패: %s"),
			*TargetBuilding->GetName()
		);
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("[Destroy] 건물 파괴 완료: %s"),
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
	if (!bIsDestroyMode)
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

	if (!TraceFromScreenCenter(HitResult))
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

bool UKOGridBuildComponent::TraceFromScreenCenter(FHitResult& OutHit) const
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
		GroundTraceChannel,
		QueryParams
	);
	
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (IsInGameThread())
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