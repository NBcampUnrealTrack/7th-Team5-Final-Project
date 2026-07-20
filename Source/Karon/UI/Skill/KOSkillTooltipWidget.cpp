// Copyright Karon Team 5. All Rights Reserved.

#include "KOSkillTooltipWidget.h"
#include "UI/Skill/KOSkillCostEntryWidget.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Data/KODataTableTypes.h"
#include "Data/Type/KOSkillTypes.h"

#include "CommonButtonBase.h"
#include "CommonTextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"
#include "Components/Border.h"

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

	if (ConfirmButton)
	{
		ConfirmButton->OnClicked().AddUObject(this, &UKOSkillTooltipWidget::HandleConfirmButtonClicked);
	}
}

void UKOSkillTooltipWidget::NativeDestruct()
{
	CachedInventoryComp = nullptr;

	Super::NativeDestruct();
}

void UKOSkillTooltipWidget::HandleConfirmButtonClicked()
{
	OnConfirmed.Broadcast();
}

void UKOSkillTooltipWidget::RefreshConfirmButtonState(ESkillState CurrentState)
{
	const bool bCanUnlock = CurrentState == ESkillState::CanUnlock;
	const bool bIsUnlocked = CurrentState == ESkillState::Unlocked;

	if (ConfirmButton)
	{
		ConfirmButton->SetIsEnabled(bCanUnlock);
		ConfirmButton->SetVisibility(
			bIsUnlocked ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}


	if (ConfirmButtonLockedOverlay)
	{
		ConfirmButtonLockedOverlay->SetVisibility(
			bIsUnlocked ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UKOSkillTooltipWidget::RefreshGearAmountText()
{
	const int32 CurrentGearAmount = (CachedInventoryComp && !CachedGearItemId.IsNone())
			? CachedInventoryComp->GetCountOf(CachedGearItemId) : 0;

	if (CurrentItemAmount)
	{
		CurrentItemAmount->SetText(FText::AsNumber(CurrentGearAmount));
	}

	if (RequireItemAmount)
	{
		RequireItemAmount->SetText(FText::AsNumber(CachedRequiredGearAmount));
	}
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
		SkillIcon->SetBrushResourceObject(IconTexture);
	}
	else if (SkillRow.Icon.IsNull() && SkillIcon)
	{
		SkillIcon->SetBrush(DefaultBrush);
	}

	RefreshConfirmButtonState(CurrentState);

	const UKOLoadSubsystem* LoadSubsystem = UKOLoadSubsystem::Get(this);
	
	CachedGearItemId = NAME_None;
	CachedRequiredGearAmount = 0;
	
	if (!SkillRow.UnlockCosts.IsEmpty())
	{
		const FSkillCost& GearCost = SkillRow.UnlockCosts[0];

		CachedGearItemId = LoadSubsystem ? LoadSubsystem->FindItemIdByTag(GearCost.ItemTag) : NAME_None;
		CachedRequiredGearAmount = GearCost.Amount;
	}

	RefreshGearAmountText();

	/*
	if (CostListContainer == nullptr || CostWidget == nullptr)
	{
		return;
	}

	CostListContainer->ClearChildren();
	CachedCostItemIds.Reset();
	
	for (const FSkillCost& CostData : SkillRow.UnlockCosts)
	{
		const FKOItemRow* MatchedItemRow = CostItemRows.FindByPredicate([&CostData](const FKOItemRow& ItemRow)
		{
			return ItemRow.ItemTag == CostData.ItemTag;
		});

		if (MatchedItemRow == nullptr) continue;

		FName ItemId = LoadSubsystem ? LoadSubsystem->FindItemIdByTag(CostData.ItemTag) : NAME_None;

		int32 CurrentAmount = (CachedInventoryComp && !ItemId.IsNone())
			                      ? CachedInventoryComp->GetCountOf(ItemId) : 0;
		
		if (CachedGearItemId.IsNone())
		{
			CachedGearItemId = ItemId;
			CachedRequiredGearAmount = CostData.Amount;
		}

		UTexture2D* ItemTexture = (LoadSubsystem && !ItemId.IsNone())
			                          ? LoadSubsystem->ResolveItemIcon(ItemId) : MatchedItemRow->Icon.Get();

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
	*/
}

void UKOSkillTooltipWidget::RefreshCostWidget(ESkillState NewCurrentState, const TArray<FKOItemRow>& CostItemRows)
{
	RefreshConfirmButtonState(NewCurrentState);

	/*
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
	*/
	RefreshGearAmountText();
}
