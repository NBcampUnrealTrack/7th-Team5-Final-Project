// Copyright Karon Team 5. All Rights Reserved.
#include "UI/KOInventorySlotWidget.h"
#include "Items/KOItemLibrary.h"

void UKOInventorySlotWidget::SetSlotData(const FKOItemSlot& InSlot)
{
	SlotData = InSlot;
	ItemDisplayName = FText::GetEmpty();
	ItemIcon = nullptr;

	if (SlotData.HasItem())
	{
		ItemDisplayName = UKOItemLibrary::GetDisplayName(this, SlotData.Kind, SlotData.ItemId);
		ItemIcon        = UKOItemLibrary::GetIcon(this, SlotData.Kind, SlotData.ItemId);
	}

	BP_OnSlotDataSet(SlotData);
}

const FKOItemSlot& UKOInventorySlotWidget::GetSlotData() const
{
	return SlotData;
}
