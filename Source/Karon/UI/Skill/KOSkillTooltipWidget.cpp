// Copyright Karon Team 5. All Rights Reserved.

#include "KOSkillTooltipWidget.h"
#include "UI/Skill/KOSkillCostEntryWidget.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Subsystem/KOLoadSubsystem.h"

#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Data/KODataTableTypes.h"
#include "Data/Type/KOSkillTypes.h"

void UKOSkillTooltipWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (APlayerController* OwningController = GetOwningPlayer())
	{
		if (UKOInventoryComponent* InventoryComponent = OwningController->FindComponentByClass<UKOInventoryComponent>())
		{
			CachedInventoryComp = InventoryComponent;
		}
	}
}

void UKOSkillTooltipWidget::NativeDestruct()
{
	CachedInventoryComp = nullptr;

	Super::NativeDestruct();
}

void UKOSkillTooltipWidget::InitializeSkillTooltipWidget(const FKOSkillRow& SkillRow, const FText& ExecutionType,
                                                         const TArray<FKOItemRow>& CostItemRows)
{
	if (SkillName && SkillClassification)
	{
		SkillName->SetText(SkillRow.DisplayName);
		SkillClassification->SetText(ExecutionType);
	}

	if (SkillExplanation)
	{
		SkillExplanation->SetText(SkillRow.Description);
	}

	UTexture2D* IconTexture = SkillRow.Icon.Get();
	if (IconTexture && SkillIcon)
	{
		FSlateBrush SlateBrush;
		SlateBrush.SetResourceObject(IconTexture);
		SkillIcon->SetBrush(SlateBrush);
	}

	if (CostListContainer == nullptr || CostWidget == nullptr)
	{
		return;
	}

	CostListContainer->ClearChildren();

	const UKOLoadSubsystem* LoadSubsystem = UKOLoadSubsystem::Get(this);

	for (const FSkillCost& CostData : SkillRow.UnlockCosts)
	{
		const FKOItemRow* MatchedItemRow = CostItemRows.FindByPredicate([&CostData](const FKOItemRow& ItemRow)
		{
			return ItemRow.ItemTag == CostData.ItemTag;
		});

		if (MatchedItemRow == nullptr) continue;

		FName ItemId = LoadSubsystem ? LoadSubsystem->FindItemIdByTag(CostData.ItemTag) : NAME_None;

		int32 CurrentAmount = (CachedInventoryComp && !ItemId.IsNone())
			? CachedInventoryComp->GetCountOf(ItemId)
			: 0;

		UTexture2D* ItemTexture = (LoadSubsystem && !ItemId.IsNone())
			? LoadSubsystem->ResolveItemIcon(ItemId)
			: MatchedItemRow->Icon.Get();

		UKOSkillCostEntryWidget* EntryWidget = CreateWidget<UKOSkillCostEntryWidget>(this, CostWidget);
		if (EntryWidget == nullptr) continue;

		EntryWidget->InitializeEntryWidget(
			ItemTexture,
			MatchedItemRow->DisplayName,
			FText::AsNumber(CurrentAmount),
			FText::AsNumber(CostData.Amount)
		);

		CostListContainer->AddChildToVerticalBox(EntryWidget);
	}
}
