#include "KOGhostPreview.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
    // 엔진 기본 Plane 메시는 한 변 100uu. 월드 크기 대비 스케일 환산에 사용.
    constexpr float GEnginePlaneSize = 100.0f;
}

AKOGhostPreview::AKOGhostPreview()
{
    PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(SceneRoot);

    SetActorEnableCollision(false);

	// 커버리지 오버레이 평면. 건물 회전/스케일에 영향받지 않도록 절대 트랜스폼 사용.
	CoverageMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoverageOverlay"));
	CoverageMeshComponent->SetupAttachment(SceneRoot);
	CoverageMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoverageMeshComponent->SetGenerateOverlapEvents(false);
	CoverageMeshComponent->SetCastShadow(false);
	CoverageMeshComponent->SetUsingAbsoluteLocation(true);
	CoverageMeshComponent->SetUsingAbsoluteRotation(true);
	CoverageMeshComponent->SetUsingAbsoluteScale(true);
	CoverageMeshComponent->SetVisibility(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMeshFinder(TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneMeshFinder.Succeeded())
	{
		CoveragePlaneMesh = PlaneMeshFinder.Object;
		CoverageMeshComponent->SetStaticMesh(CoveragePlaneMesh);
	}
	
	// 컨베이어 방향 표시 화살표.
	DirectionArrowComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DirectionArrow"));
	DirectionArrowComponent->SetupAttachment(SceneRoot);
	DirectionArrowComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DirectionArrowComponent->SetGenerateOverlapEvents(false);
	DirectionArrowComponent->SetCastShadow(false);
	DirectionArrowComponent->SetVisibility(false);
}

void AKOGhostPreview::ShowCoverageOverlay(const FVector& WorldCenter, const FVector2D& WorldSize)
{
	if (!CoverageMeshComponent || !CoveragePlaneMesh)
	{
		return;
	}

	CoverageMeshComponent->SetWorldLocationAndRotation(WorldCenter, FRotator::ZeroRotator);
	CoverageMeshComponent->SetWorldScale3D(FVector(
		WorldSize.X / GEnginePlaneSize,
		WorldSize.Y / GEnginePlaneSize,
		1.0f));
	
	if (CoverageMaterial)
	{
		CoverageMeshComponent->SetMaterial(0, CoverageMaterial);
	}

	CoverageMeshComponent->SetVisibility(true);
}

void AKOGhostPreview::HideCoverageOverlay()
{
	if (CoverageMeshComponent)
	{
		CoverageMeshComponent->SetVisibility(false);
	}
}

void AKOGhostPreview::ShowDirectionArrow(float AdditionalYaw)
{
	if (!DirectionArrowComponent)
	{
		return;
	}

	if (DirectionArrowMesh)
	{
		DirectionArrowComponent->SetStaticMesh(DirectionArrowMesh);
	}
	
	if (DirectionArrowMaterial)
	{
		DirectionArrowComponent->SetMaterial(0, DirectionArrowMaterial);
	}

	DirectionArrowComponent->SetRelativeLocation(DirectionArrowRelativeLocation);

	FRotator FinalRotation = DirectionArrowRelativeRotation;
	FinalRotation.Yaw += AdditionalYaw;

	DirectionArrowComponent->SetRelativeRotation(FinalRotation);
	DirectionArrowComponent->SetRelativeScale3D(DirectionArrowRelativeScale);

	DirectionArrowComponent->SetVisibility(true);
}

void AKOGhostPreview::HideDirectionArrow()
{
	if (DirectionArrowComponent)
	{
		DirectionArrowComponent->SetVisibility(false);
	}
}

void AKOGhostPreview::SetupFromBuildingClass(TSubclassOf<AActor> InBuildingClass)
{
	ClearPreviewMeshComponents();

	if (!InBuildingClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Preview] InBuildingClass가 없습니다."));
		return;
	}

	const AActor* DefaultBuildingActor = InBuildingClass->GetDefaultObject<AActor>();
	if (!DefaultBuildingActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Preview] DefaultBuildingActor가 없습니다."));
		return;
	}

	UBlueprintGeneratedClass* BlueprintClass = Cast<UBlueprintGeneratedClass>(InBuildingClass);
	while (BlueprintClass)
	{
		USimpleConstructionScript* SimpleConstructionScript =
			BlueprintClass->SimpleConstructionScript;

		if (SimpleConstructionScript)
		{
			const TArray<USCS_Node*>& Nodes = SimpleConstructionScript->GetAllNodes();

			for (USCS_Node* Node : Nodes)
			{
				if (!Node)
				{
					continue;
				}

				UActorComponent* ComponentTemplate =
					Node->GetActualComponentTemplate(BlueprintClass);

				USkeletalMeshComponent* SourceMeshComponent =
					Cast<USkeletalMeshComponent>(ComponentTemplate);

				if (!SourceMeshComponent)
				{
					continue;
				}

				AddPreviewMeshComponentFromTemplate(SourceMeshComponent);
			}
		}

		BlueprintClass = Cast<UBlueprintGeneratedClass>(BlueprintClass->GetSuperClass());
	}
}

void AKOGhostPreview::AddPreviewMeshComponentFromTemplate(
	const USkeletalMeshComponent* SourceMeshComponent
)
{
	if (!SourceMeshComponent)
	{
		return;
	}

	USkeletalMesh* SourceMesh = SourceMeshComponent->GetSkeletalMeshAsset();

	if (!SourceMesh)
	{
		return;
	}

	USkeletalMeshComponent* NewPreviewMeshComponent = NewObject<USkeletalMeshComponent>(this);

	if (!NewPreviewMeshComponent)
	{
		return;
	}

	NewPreviewMeshComponent->SetSkeletalMesh(SourceMesh);

	NewPreviewMeshComponent->SetRelativeTransform(
		SourceMeshComponent->GetRelativeTransform()
	);

	NewPreviewMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	NewPreviewMeshComponent->SetGenerateOverlapEvents(false);

	const int32 MaterialCount = SourceMeshComponent->GetNumMaterials();
	for (int32 Index = 0; Index < MaterialCount; ++Index)
	{
		NewPreviewMeshComponent->SetMaterial(
			Index,
			SourceMeshComponent->GetMaterial(Index)
		);
	}

	NewPreviewMeshComponent->AttachToComponent(
		RootComponent,
		FAttachmentTransformRules::KeepRelativeTransform
	);

	NewPreviewMeshComponent->RegisterComponent();

	PreviewMeshComponents.Add(NewPreviewMeshComponent);
}

void AKOGhostPreview::ClearPreviewMeshComponents()
{
    for (USkeletalMeshComponent* MeshComponent : PreviewMeshComponents)
    {
        if (MeshComponent)
        {
            MeshComponent->DestroyComponent();
        }
    }

    PreviewMeshComponents.Empty();
}