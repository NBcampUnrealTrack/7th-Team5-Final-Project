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
	EKOCraftTargetType InTargetType,
	FName InTargetId,
	const FText& InDisplayName,
	UTexture2D* InIcon,
	bool bInCanCraft)
{
	TargetType = InTargetType;
	TargetId = InTargetId;

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
		if (!bInCanCraft)
		{
			FButtonStyle NotCraftableStyle = EntryButton->GetStyle();

			NotCraftableStyle.Normal.TintColor = FSlateColor(NotCraftableColor);
			NotCraftableStyle.Hovered.TintColor = FSlateColor(NotCraftableHoveredColor);
			NotCraftableStyle.Pressed.TintColor = FSlateColor(NotCraftablePressedColor);

			EntryButton->SetStyle(NotCraftableStyle);
		}
	}
}

void UKOFactoryCraftEntryWidget::HandleClicked()
{
	if (!TargetId.IsNone())
	{
		OnClicked.Broadcast(TargetType, TargetId);
	}
}