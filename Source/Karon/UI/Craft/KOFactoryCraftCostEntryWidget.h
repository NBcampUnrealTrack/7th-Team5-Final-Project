#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KOFactoryCraftCostEntryWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;

UCLASS()
class KARON_API UKOFactoryCraftCostEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetupCost(const FText& InItemName, UTexture2D* InIcon, int32 InOwnedCount, int32 InRequiredCount);

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ItemIconImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ItemNameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CountText;
};