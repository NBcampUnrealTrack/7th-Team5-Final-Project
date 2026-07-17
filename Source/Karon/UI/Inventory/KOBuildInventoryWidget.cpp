#include "KOBuildInventoryWidget.h"

#include "Groups/CommonButtonGroupBase.h"
#include "CommonButtonBase.h"
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
		Button_Inventory->SetIsFocusable(false);
		Button_Inventory->OnClicked().AddUObject(this, &ThisClass::HandleInventoryClicked);
	}

	if (Button_Factory)
	{
		Button_Factory->SetIsFocusable(false);
		Button_Factory->OnClicked().AddUObject(this, &ThisClass::HandleFactoryClicked);
	}
	
	// 버큰 그룹 등록
	TabButtonGroup = NewObject<UCommonButtonGroupBase>(this);

	if (TabButtonGroup)
	{
		TabButtonGroup->SetSelectionRequired(true);

		TabButtonGroup->AddWidget(Button_Inventory);
		TabButtonGroup->AddWidget(Button_Factory);
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
