#include "UI/Craft/KOFactoryCraftEntryWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UKOFactoryCraftEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	CacheDefaultButtonStyle();

	if (EntryButton)
	{
		EntryButton->OnClicked.AddDynamic(this, &UKOFactoryCraftEntryWidget::HandleClicked);
	}
	
	RefreshVisualState();
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
	bool bInCanCraft,
	int32 InOwnedCount)
{
	TargetType = InTargetType;
	TargetId = InTargetId;
	bCanCraft = bInCanCraft;

	if (FactoryNameText)
	{
		FactoryNameText->SetText(InDisplayName);
	}

	if (FactoryIconImage)
	{
		FactoryIconImage->SetBrushFromTexture(InIcon);
		FactoryIconImage->SetVisibility(InIcon ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
	
	if (TextAmount)
	{
		TextAmount->SetText(FText::AsNumber(InOwnedCount));
	}
	
	if (NotCraftableImage)
	{
		NotCraftableImage->SetVisibility(bCanCraft
			? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
	
	CacheDefaultButtonStyle();
	RefreshVisualState();
}

void UKOFactoryCraftEntryWidget::SetSelected(bool bInSelected)
{
	if (bSelected == bInSelected)
	{
		return;
	}

	bSelected = bInSelected;
	RefreshVisualState();
}

bool UKOFactoryCraftEntryWidget::MatchesTarget(EKOCraftTargetType InTargetType, FName InTargetId) const
{
	return TargetType == InTargetType && TargetId == InTargetId;
}

void UKOFactoryCraftEntryWidget::HandleClicked()
{
	if (!TargetId.IsNone())
	{
		OnClicked.Broadcast(TargetType, TargetId);
	}
}

void UKOFactoryCraftEntryWidget::CacheDefaultButtonStyle()
{
	if (bDefaultStyleCached || !EntryButton)
	{
		return;
	}

	DefaultButtonStyle = EntryButton->GetStyle();
	bDefaultStyleCached = true;
}

void UKOFactoryCraftEntryWidget::RefreshVisualState()
{
	CacheDefaultButtonStyle();

	if (EntryButton && bDefaultStyleCached)
	{
		// BP에 설정한 원래 스타일에서 시작
		FButtonStyle NewStyle = DefaultButtonStyle;

		// 선택된 경우 이미지 교체
		if (bSelected && SelectedNormalImage)
		{
			NewStyle.Normal.SetResourceObject(SelectedNormalImage);
			NewStyle.Hovered.SetResourceObject(SelectedNormalImage);
			NewStyle.Pressed.SetResourceObject(SelectedNormalImage);
		}

		EntryButton->SetStyle(NewStyle);
	}

	if (SelectionArrowText)
	{
		SelectionArrowText->SetVisibility(bSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}
