// Copyright Karon Team 5. All Rights Reserved.

#include "KOSkillCostEntryWidget.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"

void UKOSkillCostEntryWidget::InitializeEntryWidget(UTexture2D* ItemTexture2D, const FText& ItemName,
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
	float fRequireAmount  = FCString::Atof(*CleanRequireAmount);
	
	if (fCurrentAmount < fRequireAmount)
	{
		CurrentItemAmount->SetColorAndOpacity(FLinearColor::Red);
	}
	else
	{
		CurrentItemAmount->SetColorAndOpacity(FLinearColor::White);
	}
}
