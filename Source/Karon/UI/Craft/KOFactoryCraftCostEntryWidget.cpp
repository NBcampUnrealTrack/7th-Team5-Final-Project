#include "KOFactoryCraftCostEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UKOFactoryCraftCostEntryWidget::SetupCost(
	const FText& InItemName,
	UTexture2D* InIcon,
	int32 InOwnedCount,
	int32 InRequiredCount)
{
	if (ItemNameText)
	{
		ItemNameText->SetText(InItemName);
	}

	if (ItemIconImage)
	{
		ItemIconImage->SetBrushFromTexture(InIcon);
		ItemIconImage->SetVisibility(InIcon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	if (CountText)
	{
		CountText->SetText(FText::FromString(
			FString::Printf(TEXT("%d / %d"), InOwnedCount, InRequiredCount)
		));

		const bool bEnough = InOwnedCount >= InRequiredCount;
		CountText->SetColorAndOpacity(
			bEnough
				? FSlateColor(FLinearColor::White)
				: FSlateColor(FLinearColor::Red)
		);
	}
}