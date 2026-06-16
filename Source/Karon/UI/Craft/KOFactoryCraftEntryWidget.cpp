#include "UI/Craft/KOFactoryCraftEntryWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UKOFactoryCraftEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EntryButton)
	{
		EntryButton->OnClicked.AddDynamic(this, &UKOFactoryCraftEntryWidget::HandleClicked);
	}
}

void UKOFactoryCraftEntryWidget::NativeDestruct()
{
	if (EntryButton)
	{
		EntryButton->OnClicked.RemoveDynamic(this, &UKOFactoryCraftEntryWidget::HandleClicked);
	}

	Super::NativeDestruct();
}

void UKOFactoryCraftEntryWidget::SetupEntry(
	FName InFactoryId,
	const FText& InDisplayName,
	UTexture2D* InIcon,
	bool bInCanCraft)
{
	FactoryId = InFactoryId;

	if (FactoryNameText)
	{
		FactoryNameText->SetText(InDisplayName);
	}

	if (FactoryIconImage)
	{
		FactoryIconImage->SetBrushFromTexture(InIcon);
		FactoryIconImage->SetVisibility(InIcon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	
	if (EntryButton)
	{
		const FLinearColor TargetColor =
			bInCanCraft ? CraftableColor : NotCraftableColor;

		EntryButton->SetBackgroundColor(TargetColor);
	}
}

void UKOFactoryCraftEntryWidget::HandleClicked()
{
	if (!FactoryId.IsNone())
	{
		OnClicked.Broadcast(FactoryId);
	}
}