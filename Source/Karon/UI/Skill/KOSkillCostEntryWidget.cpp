// Copyright Karon Team 5. All Rights Reserved.

#include "KOSkillCostEntryWidget.h"
#include "Data/Type/KOSkillTypes.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"

void UKOSkillCostEntryWidget::InitializeEntryWidget(UTexture2D* ItemTexture2D, const FText& ItemName,
                                                    ESkillState CurrentState,
                                                    const FText& CurrentAmount, const FText& RequireAmount)
{
	if (RequireItemName && CurrentItemAmount && RequireItemAmount)
	{
		RequireItemName->SetText(ItemName);
		CurrentItemAmount->SetText(CurrentAmount);
		RequireItemAmount->SetText(RequireAmount);
	}

	if (ItemIcon)
	{
		FSlateBrush SlateBrush;
		SlateBrush.SetResourceObject(ItemTexture2D);
		ItemIcon->SetBrush(SlateBrush);
	}

	FString CleanCurrentAmount = *CurrentAmount.ToString().Replace(TEXT(","), TEXT(""));
	FString CleanRequireAmount = *RequireAmount.ToString().Replace(TEXT(","), TEXT(""));

	float fCurrentAmount = FCString::Atof(*CleanCurrentAmount);
	CachedRequireAmount = FCString::Atof(*CleanRequireAmount);

	if (CurrentState == ESkillState::Unlocked)
	{
		CurrentItemAmount->SetColorAndOpacity(FLinearColor::White);
	}
	else
	{
		if (fCurrentAmount < CachedRequireAmount)
		{
			CurrentItemAmount->SetColorAndOpacity(FLinearColor::Red);
		}
		else
		{
			CurrentItemAmount->SetColorAndOpacity(FLinearColor::White);
		}
	}
}

void UKOSkillCostEntryWidget::RefreshEntryWidget(ESkillState NewCurrentState, const FText& NewCurrentAmount)
{
	if (CurrentItemAmount)
	{
		CurrentItemAmount->SetText(NewCurrentAmount);

		FString CleanCurrentAmount = *NewCurrentAmount.ToString().Replace(TEXT(","), TEXT(""));

		float fCurrentAmount = FCString::Atof(*CleanCurrentAmount);

		if (NewCurrentState == ESkillState::Unlocked)
		{
			CurrentItemAmount->SetColorAndOpacity(FLinearColor::White);
		}
		else
		{
			if (fCurrentAmount < CachedRequireAmount)
			{
				CurrentItemAmount->SetColorAndOpacity(FLinearColor::Red);
			}
			else
			{
				CurrentItemAmount->SetColorAndOpacity(FLinearColor::White);
			}
		}
	}
}
