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

void UKOSkillTooltipWidget::InitializeSkillTooltipWidget(const FKOSkillRow& SkillRow, ESkillState CurrentState,
                                                         const FText& ExecutionType,
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
	else if (SkillRow.Icon.IsNull() && SkillIcon)
	{
		SkillIcon->SetBrush(DefaultBrush);
	}

	if (CostListContainer == nullptr || CostWidget == nullptr)
	{
		return;
	}

	CostListContainer->ClearChildren();
	CachedCostItemIds.Reset();

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
			CurrentState,
			FText::AsNumber(CurrentAmount),
			FText::AsNumber(CostData.Amount)
		);

		CostListContainer->AddChildToVerticalBox(EntryWidget);
		CachedCostItemIds.Add(ItemId);
	}
}

void UKOSkillTooltipWidget::RefreshCostWidget(ESkillState NewCurrentState, const TArray<FKOItemRow>& CostItemRows)
{
	if (CostListContainer == nullptr)
	{
		return;
	}

	const int32 ChildCount = CostListContainer->GetChildrenCount();
	for (int32 i = 0; i < ChildCount && i < CachedCostItemIds.Num(); ++i)
	{
		UKOSkillCostEntryWidget* EntryWidget = Cast<UKOSkillCostEntryWidget>(CostListContainer->GetChildAt(i));
		if (EntryWidget == nullptr) continue;

		const int32 CurrentAmount = (CachedInventoryComp && !CachedCostItemIds[i].IsNone())
			                            ? CachedInventoryComp->GetCountOf(CachedCostItemIds[i])
			                            : 0;

		EntryWidget->RefreshEntryWidget(NewCurrentState, FText::AsNumber(CurrentAmount));
	}
}
