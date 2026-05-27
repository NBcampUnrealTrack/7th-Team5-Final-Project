#include "KOBuildAssignEntryWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "InputCoreTypes.h"
#include "KOBuildDragDropOperation.h"

void UKOBuildAssignEntryWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	RefreshVisual();
}

void UKOBuildAssignEntryWidget::SetupEntry(
	FName InFactoryId,
	FText InDisplayName,
	UTexture2D* InIcon
)
{
	FactoryId = InFactoryId;
	DisplayName = InDisplayName;
	Icon = InIcon;

	RefreshVisual();
}

void UKOBuildAssignEntryWidget::RefreshVisual()
{
	if (DisplayNameText)
	{
		DisplayNameText->SetText(
			DisplayName.IsEmpty()
				? FText::FromName(FactoryId)
				: DisplayName
		);
	}

	if (IconImage && Icon)
	{
		IconImage->SetBrushFromTexture(Icon);
	}
}

FReply UKOBuildAssignEntryWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent
)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(
			InMouseEvent,
			this,
			EKeys::LeftMouseButton
		);

		return Reply.NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UKOBuildAssignEntryWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation
)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (FactoryId.IsNone())
	{
		return;
	}

	UKOBuildDragDropOperation* DragOperation = NewObject<UKOBuildDragDropOperation>(this);

	if (!DragOperation)
	{
		return;
	}

	DragOperation->FactoryId = FactoryId;
	DragOperation->DisplayName = DisplayName;
	DragOperation->Icon = Icon;
	
	// 마우스를 따라다닐 드래그 아이콘 위젯
	if (Icon)
	{
		UImage* DragVisualImage = NewObject<UImage>(DragOperation);

		if (DragVisualImage)
		{
			DragVisualImage->SetBrushFromTexture(Icon);
			DragVisualImage->SetBrushSize(DragVisualSize);
			DragVisualImage->SetOpacity(DragVisualOpacity);

			DragOperation->DefaultDragVisual = DragVisualImage;
		}
	}
	
	DragOperation->Pivot = EDragPivot::CenterCenter;

	OutOperation = DragOperation;
}