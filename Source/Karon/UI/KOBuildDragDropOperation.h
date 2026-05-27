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
	
	// 갯수 추가(설비도 아이템으로 갯수 있음)
};