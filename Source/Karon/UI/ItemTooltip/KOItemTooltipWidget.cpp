#include "KOItemTooltipWidget.h"

#include "Components/TextBlock.h"
#include "Items/KOItemLibrary.h"

void UKOItemTooltipWidget::SetSlot(EKOSlotKind Kind, FName Id)
{
	if (Id.IsNone())
	{
		if (NameText)
		{
			NameText->SetText(FText::GetEmpty());
		}

		if (DescriptionText)
		{
			DescriptionText->SetText(FText::GetEmpty());
		}

		return;
	}

	if (NameText)
	{
		NameText->SetText(
			UKOItemLibrary::GetDisplayName(this, Kind, Id)
		);
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(
			UKOItemLibrary::GetDescription(this, Kind, Id)
		);
	}
}