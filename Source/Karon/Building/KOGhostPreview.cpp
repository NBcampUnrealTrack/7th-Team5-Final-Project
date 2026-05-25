#include "KOGhostPreview.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "GameFramework/Actor.h"

AKOGhostPreview::AKOGhostPreview()
{
    PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	SetRootComponent(SceneRoot);

    SetActorEnableCollision(false);
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

				UStaticMeshComponent* SourceMeshComponent =
					Cast<UStaticMeshComponent>(ComponentTemplate);

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
	const UStaticMeshComponent* SourceMeshComponent
)
{
	if (!SourceMeshComponent)
	{
		return;
	}

	UStaticMesh* SourceMesh = SourceMeshComponent->GetStaticMesh();

	if (!SourceMesh)
	{
		return;
	}

	UStaticMeshComponent* NewPreviewMeshComponent =
		NewObject<UStaticMeshComponent>(this);

	if (!NewPreviewMeshComponent)
	{
		return;
	}

	NewPreviewMeshComponent->SetStaticMesh(SourceMesh);

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
    for (UStaticMeshComponent* MeshComponent : PreviewMeshComponents)
    {
        if (MeshComponent)
        {
            MeshComponent->DestroyComponent();
        }
    }

    PreviewMeshComponents.Empty();
}