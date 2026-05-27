#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "KOBuildDragDropOperation.generated.h"

class UTexture2D;

UCLASS()
class KARON_API UKOBuildDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Build|Drag")
	FName FactoryId = NAME_None;

	UPROPERTY(BlueprintReadWrite, Category = "Build|Drag")
	FText DisplayName;

	UPROPERTY(BlueprintReadWrite, Category = "Build|Drag")
	TObjectPtr<UTexture2D> Icon = nullptr;
};