#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOGhostPreview.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS()
class KARON_API AKOGhostPreview : public AActor
{
	GENERATED_BODY()

public:
	AKOGhostPreview();

	void SetupFromBuildingClass(TSubclassOf<AActor> InBuildingClass);

private:
	void ClearPreviewMeshComponents();
	
	void AddPreviewMeshComponentFromTemplate(
		const UStaticMeshComponent* SourceMeshComponent
	);

private:
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USceneComponent> PreviewRoot;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> PreviewMeshComponents;
};