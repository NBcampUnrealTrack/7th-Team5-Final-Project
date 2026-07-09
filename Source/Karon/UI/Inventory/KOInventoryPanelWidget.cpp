// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Inventory/KOInventoryPanelWidget.h"

#include "Component/Inventory/KOInventoryComponent.h"
#include "UI/Inventory/KOInventoryWidget.h"
#include "Blueprint/WidgetTree.h"
#include "UI/Inventory/KOEquipmentSlotWidget.h"

UKOInventoryPanelWidget::UKOInventoryPanelWidget()
{
    InputMode = EKOUIInputMode::All;

    // Back(ESC) 입력 시 자동으로 Deactivate되어 스택에서 닫힌다. (토글 제거 → Back 일원화)
    bIsBackHandler = true;
}

void UKOInventoryPanelWidget::SetInventoryComponent(UKOInventoryComponent* InComponent)
{    
    if (InventoryWidget)
    {
        InventoryWidget->SetInventoryComponent(InComponent);
    }
}

void UKOInventoryPanelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (InventoryWidget)
    {
        InventoryWidget->OnSlotClicked.AddDynamic(this, &UKOInventoryPanelWidget::HandleSlotClicked);
        InventoryWidget->OnSlotRightClicked.AddDynamic(this, &UKOInventoryPanelWidget::HandleSlotRightClicked);
    }

    if (BuildQuickSlotBar)
    {
        BuildQuickSlotBar->SetDisplayMode(EKOQuickSlotBarDisplayMode::Inventory);
    }

    CacheEquipmentSlotWidgets();
}

void UKOInventoryPanelWidget::NativeDestruct()
{
    if (InventoryWidget)
    {
        InventoryWidget->OnSlotClicked.RemoveDynamic(this, &UKOInventoryPanelWidget::HandleSlotClicked);
        InventoryWidget->OnSlotRightClicked.RemoveDynamic(this, &UKOInventoryPanelWidget::HandleSlotRightClicked);
    }

    Super::NativeDestruct();
}

void UKOInventoryPanelWidget::HandleSlotClicked(int32 SlotIndex, const FKOItemSlot& InSlot)
{
    OnSlotClicked(SlotIndex, InSlot);
}

void UKOInventoryPanelWidget::HandleSlotRightClicked(int32 SlotIndex, const FKOItemSlot& InSlot)
{
    TryEquipItemToMatchingSlot(SlotIndex, InSlot);
}

void UKOInventoryPanelWidget::CacheEquipmentSlotWidgets()
{
    EquipmentSlotWidgets.Reset();

    if (!WidgetTree)
    {
        return;
    }

    WidgetTree->ForEachWidget([this](UWidget* Widget)
    {
        if (UKOEquipmentSlotWidget* EquipmentSlot = Cast<UKOEquipmentSlotWidget>(Widget))
        {
            EquipmentSlotWidgets.Add(EquipmentSlot);
        }
    });
}

bool UKOInventoryPanelWidget::TryEquipItemToMatchingSlot(int32 SlotIndex, const FKOItemSlot& InSlot)
{
    if (InSlot.Kind != EKOSlotKind::Item || InSlot.ItemId.IsNone())
    {
        return false;
    }

    for (UKOEquipmentSlotWidget* EquipmentSlot : EquipmentSlotWidgets)
    {
        if (!EquipmentSlot)
        {
            continue;
        }

        if (EquipmentSlot->TryEquipItemFromInventorySlot(SlotIndex, InSlot.ItemId))
        {
            return true;
        }
    }

    return false;
}

void UKOInventoryPanelWidget::OnSlotClicked_Implementation(int32 SlotIndex, const FKOItemSlot& InSlot)
{
    // 기본 동작 없음. 자식 클래스/BP에서 오버라이드.
}
