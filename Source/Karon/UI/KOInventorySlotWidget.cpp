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
		ItemDisplayName = UKOItemLibrary::GetItemDisplayName(this, SlotData.ItemId);
		ItemIcon        = UKOItemLibrary::GetItemIcon(this, SlotData.ItemId);
	}

	BP_OnSlotDataSet(SlotData);
}

const FKOItemSlot& UKOInventorySlotWidget::GetSlotData() const
{
	return SlotData;
}
