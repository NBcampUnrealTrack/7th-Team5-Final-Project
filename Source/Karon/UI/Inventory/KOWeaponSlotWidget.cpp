// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Inventory/KOWeaponSlotWidget.h"

#include "Blueprint/DragDropOperation.h"
#include "Components/Image.h"
#include "InputCoreTypes.h"

#include "Component/Inventory/KOInventoryComponent.h"
#include "UI/Inventory/KOItemDragDropOperation.h"
#include "UI/Inventory/KOItemDragSource.h"
#include "UI/ItemTooltip/KOItemTooltipWidget.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"

void UKOWeaponSlotWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ResolveInventoryComponentIfNeeded();

    RefreshVisual();
}

void UKOWeaponSlotWidget::SetInventoryComponent(UKOInventoryComponent* InInventory)
{
    InventoryComponent = InInventory;
}

void UKOWeaponSlotWidget::ResolveInventoryComponentIfNeeded()
{
    if (InventoryComponent)
    {
        return;
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    InventoryComponent = PC->FindComponentByClass<UKOInventoryComponent>();

    if (!InventoryComponent)
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            InventoryComponent = Pawn->FindComponentByClass<UKOInventoryComponent>();
        }
    }
}

bool UKOWeaponSlotWidget::NativeOnDrop(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    UKOInventoryComponent* Inventory = InventoryComponent;
    if (!Inventory)
    {
        return false;
    }

    if (!InOperation)
    {
        return false;
    }

    UKOItemDragDropOperation* ItemDragOperation = Cast<UKOItemDragDropOperation>(InOperation);
    if (!ItemDragOperation)
    {
        return false;
    }

    if (!ItemDragOperation->HasItem())
    {
        return false;
    }

    if (ItemDragOperation->IsFactory())
    {
        return false;
    }

    if (!ItemDragOperation->Source)
    {
        return false;
    }

    const FName DraggedItemId = ItemDragOperation->GetItemId();

    if (DraggedItemId.IsNone())
    {
        return false;
    }
    
    const bool bIsWeaponEquipment = UKOItemLibrary::IsWeaponEquipmentItem(this, DraggedItemId);
    if (!bIsWeaponEquipment)
    {
        return false;
    }

    const int32 Extracted = ItemDragOperation->Source->Extract(DraggedItemId, 1);
    if (Extracted <= 0)
    {
        return false;
    }
        
    // 무기 교체
    if (!EquippedWeaponId.IsNone())
    {
        const int32 Remaining = Inventory->TryAddItem(EKOSlotKind::Item, EquippedWeaponId, 1);

        if (Remaining > 0)
        {
            ItemDragOperation->Source->Restore(DraggedItemId, Extracted);
            return false;
        }

        EquippedWeaponId = NAME_None;
    }

    EquippedWeaponId = DraggedItemId;

    RefreshVisual();

    return true;
}

FReply UKOWeaponSlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        if (UnequipWeapon())
        {
            return FReply::Handled();
        }
    }
    
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

bool UKOWeaponSlotWidget::UnequipWeapon()
{
    ResolveInventoryComponentIfNeeded();
    
    if (EquippedWeaponId.IsNone())
    {
        return false;
    }

    UKOInventoryComponent* Inventory = InventoryComponent;

    if (!Inventory)
    {
        return false;
    }

    const int32 Remaining = Inventory->TryAddItem(EKOSlotKind::Item, EquippedWeaponId, 1);
    if (Remaining > 0)
    {
        return false;
    }

    EquippedWeaponId = NAME_None;

    RefreshVisual();

    return true;
}

void UKOWeaponSlotWidget::RefreshVisual()
{
    const bool bHasWeapon = !EquippedWeaponId.IsNone();

    if (WeaponIconImage)
    {
        if (bHasWeapon)
        {
            UTexture2D* Icon = UKOItemLibrary::GetIcon(this, EKOSlotKind::Item, EquippedWeaponId);
            WeaponIconImage->SetBrushFromTexture(Icon);
            WeaponIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }
        else
        {
            WeaponIconImage->SetBrushFromTexture(nullptr);
            WeaponIconImage->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
    
    RefreshTooltip();
}

void UKOWeaponSlotWidget::RefreshTooltip()
{
    if (EquippedWeaponId.IsNone())
    {
        SetToolTip(nullptr);
        return;
    }

    if (!ItemTooltipWidgetClass)
    {
        SetToolTip(nullptr);
        return;
    }

    UKOItemTooltipWidget* TooltipWidget =
        CreateWidget<UKOItemTooltipWidget>(GetOwningPlayer(), ItemTooltipWidgetClass);

    if (!TooltipWidget)
    {
        SetToolTip(nullptr);
        return;
    }

    TooltipWidget->SetSlot(EKOSlotKind::Item, EquippedWeaponId);

    SetToolTip(TooltipWidget);
}
