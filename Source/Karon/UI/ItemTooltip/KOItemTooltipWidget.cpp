#include "KOItemTooltipWidget.h"

#include "Components/TextBlock.h"
#include "Items/KOItemLibrary.h"
#include "AbilitySystem/Tag/Item/KOGameplayTags_Item.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "GameFramework/PlayerController.h"

void UKOItemTooltipWidget::SetSlot(EKOSlotKind Kind, FName Id)
{
	if (Id.IsNone())
	{
		if (NameText)
		{
			NameText->SetText(FText::GetEmpty());
		}

		if (DescriptionText)
		{
			DescriptionText->SetText(FText::GetEmpty());
		}
		
		if (TxtCategory)
		{
			TxtCategory->SetText(FText::GetEmpty());
		}

		if (TxtAmount)
		{
			TxtAmount->SetText(FText::GetEmpty());
		}

		return;
	}

	if (NameText)
	{
		NameText->SetText(UKOItemLibrary::GetDisplayName(this, Kind, Id));
	}

	if (DescriptionText)
	{
		DescriptionText->SetText(UKOItemLibrary::GetDescription(this, Kind, Id));
	}
	
	if (TxtCategory)
	{
		TxtCategory->SetText(ResolveCategoryText(Kind, Id));
	}

	if (TxtAmount)
	{
		const int32 OwnedCount = ResolveOwnedCount(Id);

		TxtAmount->SetText(FText::Format(NSLOCTEXT("KOItemTooltip", "OwnedCountFormat", "보유 {0}"),
				FText::AsNumber(OwnedCount)
			)
		);
	}
}

FText UKOItemTooltipWidget::ResolveCategoryText(EKOSlotKind Kind, FName Id) const
{
	if (Kind == EKOSlotKind::Factory)
	{
		return NSLOCTEXT("Karon", "FacilityLabel", "설비");
	}

	// 아이템 슬롯이 아니면 표시하지 않음
	if (Kind != EKOSlotKind::Item)
	{
		return FText::GetEmpty();
	}

	const FKOItemRow* ItemRow = UKOItemLibrary::GetItemRow(this, Id);
	
	if (!ItemRow)
	{
		return FText::GetEmpty();
	}
	
	if (ItemRow->Categories.HasTagExact(KOGameplayTags::Item_Category_Module))
	{
		return NSLOCTEXT("Karon", "Module", "모듈");
	}
	
	if (ItemRow->Categories.HasTagExact(KOGameplayTags::Item_Category_Resource))
	{
		return NSLOCTEXT("Karon", "Resource", "자원");
	}
	
	if (ItemRow->Categories.HasTagExact(KOGameplayTags::Item_Category_Material))
	{
		return NSLOCTEXT("Karon", "Ingredients", "가공 재료");
	}

	if (ItemRow->Categories.HasTagExact(KOGameplayTags::Item_Category_Equipment))
	{
		return NSLOCTEXT("Karon", "Armor", "장비");
	}

	if (ItemRow->Categories.HasTagExact(KOGameplayTags::Item_Category_Consumable))
	{
		return NSLOCTEXT("Karon", "Consumables", "소모품");
	}
	
	if (ItemRow->Categories.HasTagExact(KOGameplayTags::Item_Category_EnergyResource))
	{
		return NSLOCTEXT("Karon", "EnergyResource", "에너지 자원");
	}
	
	if (ItemRow->Categories.HasTagExact(KOGameplayTags::Item_Category_Core))
	{
		return NSLOCTEXT("Karon", "Core", "코어");
	}

	return NSLOCTEXT("Karon", "Others", "기타");
}

int32 UKOItemTooltipWidget::ResolveOwnedCount(FName Id) const
{
	if (Id.IsNone())
	{
		return 0;
	}

	APlayerController* PlayerController = GetOwningPlayer();

	if (!PlayerController)
	{
		return 0;
	}

	UKOInventoryComponent* Inventory = PlayerController->FindComponentByClass<UKOInventoryComponent>();

	return Inventory ? Inventory->GetCountOf(Id) : 0;
}
