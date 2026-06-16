#include "KORecipeTooltipWidget.h"

#include "Components/TextBlock.h"
#include "Data/KODataTableTypes.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
#include "Subsystem/KOLoadSubsystem.h"

void UKORecipeTooltipWidget::SetRecipe(FName RecipeId)
{
	if (!RecipeText)
	{
		return;
	}

	RecipeText->SetText(FText::GetEmpty());

	if (RecipeId.IsNone())
	{
		return;
	}

	const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
	const FKORecipeRow* Recipe = LoadSub ? LoadSub->FindRecipeRow(RecipeId) : nullptr;
	if (!LoadSub || !Recipe)
	{
		return;
	}

	auto MakeItemListText = [this, LoadSub](const TMap<FGameplayTag, int32>& Map)
	{
		TArray<FString> Parts;

		for (const TPair<FGameplayTag, int32>& Pair : Map)
		{
			const FName ItemId = LoadSub->FindItemIdByTag(Pair.Key);
			if (ItemId.IsNone()) continue;

			const FText ItemName = UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Item, ItemId);
			const FString NameString = ItemName.IsEmpty() ? ItemId.ToString() : ItemName.ToString();

			Parts.Add(FString::Printf(TEXT("%s x%d"), *NameString, Pair.Value));
		}

		return Parts.Num() > 0 ? FString::Join(Parts, TEXT(", ")) : FString(TEXT("-"));
	};

	const FString InputText = MakeItemListText(Recipe->Inputs);
	const FString OutputText = MakeItemListText(Recipe->Outputs);

	RecipeText->SetText(
		FText::FromString(
			FString::Printf(TEXT("%s  →  %s"), *InputText, *OutputText)));
}