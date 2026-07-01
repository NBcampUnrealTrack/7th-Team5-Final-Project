#include "KOBuildInventoryWidget.h"

#include "Components/Button.h"
#include "Components/WidgetSwitcher.h"
#include "UI/Craft/KOFactoryCraftWidget.h"

UKOBuildInventoryWidget::UKOBuildInventoryWidget()
{
	InputMode = EKOUIInputMode::All;
	bIsBackHandler = true;
}

void UKOBuildInventoryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (Button_Inventory)
	{
		Button_Inventory->IsFocusable = false;
		Button_Inventory->OnClicked.AddDynamic(this, &ThisClass::HandleInventoryClicked);
	}

	if (Button_Factory)
	{
		Button_Factory->IsFocusable = false;
		Button_Factory->OnClicked.AddDynamic(this, &ThisClass::HandleFactoryClicked);
	}
}

void UKOBuildInventoryWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	if (FactoryCraftWidget)
	{
		FactoryCraftWidget->SetHideEquipmentCraftables(true);
	}
	
	SetActiveTab(EKOBuildInventoryTab::Inventory);
}

void UKOBuildInventoryWidget::NativeOnDeactivated()
{
	if (FactoryCraftWidget)
	{
		FactoryCraftWidget->SetHideEquipmentCraftables(false);
	}
	
	Super::NativeOnDeactivated();
}

void UKOBuildInventoryWidget::SetActiveTab(EKOBuildInventoryTab Tab)
{
	if (!ContentSwitcher)
	{
		return;
	}

	int32 Index = 0;

	switch (Tab)
	{
	case EKOBuildInventoryTab::Inventory:
		Index = 0;
		break;

	case EKOBuildInventoryTab::FactoryCraft:
		Index = 1;
		break;

	default:
		Index = 0;
		break;
	}

	ContentSwitcher->SetActiveWidgetIndex(Index);
}

void UKOBuildInventoryWidget::HandleInventoryClicked()
{
	SetActiveTab(EKOBuildInventoryTab::Inventory);
}

void UKOBuildInventoryWidget::HandleFactoryClicked()
{
	SetActiveTab(EKOBuildInventoryTab::FactoryCraft);
}
