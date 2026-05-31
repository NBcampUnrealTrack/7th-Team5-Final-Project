#include "UI/Inventory/KOItemDragDropOperation.h"

#include "Components/Image.h"
#include "UI/Inventory/KOItemDragSource.h"

UKOItemDragDropOperation* UKOItemDragDropOperation::CreateItemDragOperation(
	UObject* Outer,
	const FKOItemSlot& InItemSlot,
	FText InDisplayName,
	UTexture2D* InIcon,
	const FVector2D& InDragVisualSize,
	float InDragVisualOpacity,
	UKOItemDragSource* InSource
)
{
	if (!Outer)
	{
		return nullptr;
	}

	if (!InItemSlot.HasItem())
	{
		return nullptr;
	}

	UKOItemDragDropOperation* DragOperation = NewObject<UKOItemDragDropOperation>(Outer);
	if (!DragOperation)
	{
		return nullptr;
	}

	DragOperation->ItemSlot    = InItemSlot;
	DragOperation->DisplayName = InDisplayName;
	DragOperation->Icon        = InIcon;
	DragOperation->Source      = InSource;

	if (InIcon)
	{
		UImage* DragVisualImage = NewObject<UImage>(DragOperation);
		if (DragVisualImage)
		{
			DragVisualImage->SetBrushFromTexture(InIcon);
			DragVisualImage->SetBrushSize(InDragVisualSize);
			DragVisualImage->SetOpacity(InDragVisualOpacity);
			DragOperation->DefaultDragVisual = DragVisualImage;
		}
	}

	DragOperation->Pivot = EDragPivot::CenterCenter;

	return DragOperation;
}
