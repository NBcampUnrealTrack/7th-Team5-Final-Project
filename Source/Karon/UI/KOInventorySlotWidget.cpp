// Copyright Karon Team 5. All Rights Reserved.
#include "UI/KOInventorySlotWidget.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Data/KODataTableTypes.h"
#include "Engine/GameInstance.h"

void UKOInventorySlotWidget::SetSlotData(const FKOItemSlot& InSlot)
{
	SlotData = InSlot;
	ItemDisplayName = FText::GetEmpty();
	ItemIcon = nullptr;

	if (SlotData.IsValid())
	{
		if (const UGameInstance* GI = GetGameInstance())
		{
			if (const UKOLoadSubsystem* LS = GI->GetSubsystem<UKOLoadSubsystem>())
			{
				if (const FKOItemRow* Row = LS->FindItemRow(SlotData.ItemId))
				{
					ItemDisplayName = Row->DisplayName;
					ItemIcon = LS->ResolveItemIcon(SlotData.ItemId);
				}
			}
		}
	}

	BP_OnSlotDataSet(SlotData);
}

const FKOItemSlot& UKOInventorySlotWidget::GetSlotData() const
{
	return SlotData;
}
