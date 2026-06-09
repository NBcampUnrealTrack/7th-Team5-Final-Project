#include "KOItemTooltipWidget.h"

#include "Components/TextBlock.h"
#include "Items/KOItemLibrary.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Data/KODataTableTypes.h"

void UKOItemTooltipWidget::SetSlot(EKOSlotKind Kind, FName Id)
{
	if (Id.IsNone())
	{
		return;
	}

	if (NameText)
	{
		NameText->SetText(
			UKOItemLibrary::GetDisplayName(this, Kind, Id));
	}

	FText Description = FText::GetEmpty();

	const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
	if (LoadSub)
	{
		if (Kind == EKOSlotKind::Item)
		{
			if (const FKOItemRow* Row = LoadSub->FindItemRow(Id))
			{
				Description = Row->Description;
			}
		}
		else if (Kind == EKOSlotKind::Factory)
		{
			if (const FKOFactoryRow* Row = LoadSub->FindFactoryRow(Id))
			{
				Description = Row->Description;
			}
		}
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(Description);
	}
}