#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/KOItemSlot.h"
#include "KOItemTooltipWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class KARON_API UKOItemTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetSlot(EKOSlotKind Kind, FName Id);

protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DescriptionText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TxtCategory;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TxtAmount;
	
private:
	FText ResolveCategoryText(EKOSlotKind Kind, FName Id) const;
	int32 ResolveOwnedCount(FName Id) const;
};