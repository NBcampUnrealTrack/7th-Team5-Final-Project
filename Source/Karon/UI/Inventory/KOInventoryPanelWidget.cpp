// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Inventory/KOInventoryPanelWidget.h"
#include "UI/Inventory/KOInventoryWidget.h"

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
    }
}

void UKOInventoryPanelWidget::NativeDestruct()
{
    if (InventoryWidget)
    {
        InventoryWidget->OnSlotClicked.RemoveDynamic(this, &UKOInventoryPanelWidget::HandleSlotClicked);
    }

    Super::NativeDestruct();
}

void UKOInventoryPanelWidget::HandleSlotClicked(int32 SlotIndex, const FKOItemSlot& InSlot)
{
    OnSlotClicked(SlotIndex, InSlot);
}

void UKOInventoryPanelWidget::OnSlotClicked_Implementation(int32 SlotIndex, const FKOItemSlot& InSlot)
{
    // 기본 동작 없음. 자식 클래스/BP에서 오버라이드.
}
