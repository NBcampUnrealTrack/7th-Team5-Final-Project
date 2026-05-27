#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KOBuildAssignEntryWidget.generated.h"

class UTextBlock;
class UImage;
class UTexture2D;
class UDragDropOperation;

UCLASS()
class KARON_API UKOBuildAssignEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Build|AssignEntry")
	void SetupEntry(FName InFactoryId, FText InDisplayName, UTexture2D* InIcon);

protected:
	virtual void NativePreConstruct() override;

	virtual FReply NativeOnMouseButtonDown(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent
	) override;

	virtual void NativeOnDragDetected(
		const FGeometry& InGeometry,
		const FPointerEvent& InMouseEvent,
		UDragDropOperation*& OutOperation
	) override;

private:
	void RefreshVisual();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|AssignEntry", meta = (ExposeOnSpawn = "true"))
	FName FactoryId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|AssignEntry", meta = (ExposeOnSpawn = "true"))
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|AssignEntry", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UTexture2D> Icon = nullptr;
	
	// 아이콘 크기
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|Drag")
	FVector2D DragVisualSize = FVector2D(64.0f, 64.0f);

	// 아이콘 투명도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|Drag")
	float DragVisualOpacity = 0.85f;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DisplayNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> IconImage;
};