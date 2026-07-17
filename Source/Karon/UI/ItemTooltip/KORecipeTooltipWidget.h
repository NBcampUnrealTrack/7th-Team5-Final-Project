#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KORecipeTooltipWidget.generated.h"

class UTextBlock;

UCLASS()
class KARON_API UKORecipeTooltipWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetRecipe(FName RecipeId);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RecipeinputText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> RecipeoutputText;
};