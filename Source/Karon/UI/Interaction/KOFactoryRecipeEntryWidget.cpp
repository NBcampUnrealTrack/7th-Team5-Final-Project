// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOFactoryRecipeEntryWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"

void UKOFactoryRecipeEntryWidget::SetRecipe(FName InRecipeId, const FText& InDisplayName)
{
    RecipeId = InRecipeId;
    if (NameText)
    {
        NameText->SetText(InDisplayName);
    }
}

void UKOFactoryRecipeEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (RecipeButton && !RecipeButton->OnClicked.IsAlreadyBound(this, &UKOFactoryRecipeEntryWidget::HandleButtonClicked))
    {
        RecipeButton->OnClicked.AddDynamic(this, &UKOFactoryRecipeEntryWidget::HandleButtonClicked);
    }
}

void UKOFactoryRecipeEntryWidget::NativeDestruct()
{
    if (RecipeButton)
    {
        RecipeButton->OnClicked.RemoveDynamic(this, &UKOFactoryRecipeEntryWidget::HandleButtonClicked);
    }
    Super::NativeDestruct();
}

void UKOFactoryRecipeEntryWidget::HandleButtonClicked()
{
    OnRecipeClicked.Broadcast(RecipeId);
}
