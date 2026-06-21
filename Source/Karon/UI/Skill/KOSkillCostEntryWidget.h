// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "KOSkillCostEntryWidget.generated.h"

class UCommonTextBlock;
class UImage;
class UTexture2D;
/**
 * 
 */
UCLASS()
class KARON_API UKOSkillCostEntryWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void InitializeEntryWidget(UTexture2D* ItemTexture2D, const FText& ItemName,
	                           const FText& CurrentAmount, const FText& RequireAmount);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ItemIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> RequireItemName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> CurrentItemAmount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> RequireItemAmount;
};
