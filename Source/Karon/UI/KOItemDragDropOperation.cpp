#include "KOItemDragDropOperation.h"

#include "Components/Image.h"

UKOItemDragDropOperation* UKOItemDragDropOperation::CreateItemDragOperation(
	UObject* Outer,
	const FKOItemSlot& InItemSlot,
	FText InDisplayName,
	UTexture2D* InIcon,
	const FVector2D& InDragVisualSize,
	float InDragVisualOpacity,
	int32 InSourceSlotIndex,
	UKOInventoryComponent* InSourceInventoryComponent,
	UKOFactoryProcessorComponent* InSourceProcessor,
	bool bInSourceFromInputBuffer
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

	// 드래그 아이템 정보
	DragOperation->ItemSlot = InItemSlot;
	DragOperation->SourceSlotIndex = InSourceSlotIndex;
	DragOperation->SourceInventoryComponent = InSourceInventoryComponent;
	DragOperation->SourceProcessor = InSourceProcessor;
	DragOperation->bSourceFromInputBuffer = bInSourceFromInputBuffer;
	DragOperation->DisplayName = InDisplayName;
	DragOperation->Icon = InIcon;

	// 마우스를 따라다닐 드래그 아이콘 위젯
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