#include "KOEquipmentSlotWidget.h"

#include "Blueprint/DragDropOperation.h"
#include "Components/Image.h"
#include "InputCoreTypes.h"

#include "Component/Inventory/KOInventoryComponent.h"
#include "Game/KOPlayerController.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Data/KODataTableTypes.h"
#include "UI/Inventory/KOItemDragDropOperation.h"
#include "UI/Inventory/KOItemDragSource.h"
#include "UI/ItemTooltip/KOItemTooltipWidget.h"

void UKOEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ResolveInventoryComponentIfNeeded();
	RefreshVisual();
}

void UKOEquipmentSlotWidget::SetInventoryComponent(UKOInventoryComponent* InInventory)
{
	InventoryComponent = InInventory;
}

void UKOEquipmentSlotWidget::ResolveInventoryComponentIfNeeded()
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

bool UKOEquipmentSlotWidget::CanAcceptItem(FName ItemId) const
{
	if (ItemId.IsNone())
	{
		return false;
	}

	const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
	if (!LoadSub)
	{
		return false;
	}

	const FKOItemRow* ItemRow = LoadSub->FindItemRow(ItemId);
	if (!ItemRow)
	{
		return false;
	}

	const FKOEquipmentRow* EquipmentRow = LoadSub->FindEquipmentRowByItemTag(ItemRow->ItemTag);

	if (!EquipmentRow)
	{
		return false;
	}

	return EquipmentRow->SlotType == SlotType;
}

bool UKOEquipmentSlotWidget::NativeOnDrop(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    ResolveInventoryComponentIfNeeded();

    UKOInventoryComponent* Inventory = InventoryComponent;
    if (!Inventory)
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

    if (!CanAcceptItem(DraggedItemId))
    {
        return false;
    }

    const int32 Extracted = ItemDragOperation->Source->Extract(DraggedItemId, 1);
    if (Extracted <= 0)
    {
        return false;
    }

	// 아이템 교체
    if (!EquippedItemId.IsNone())
    {
        const int32 Remaining = Inventory->TryAddItem(EKOSlotKind::Item, EquippedItemId, 1);

        if (Remaining > 0)
        {
            ItemDragOperation->Source->Restore(DraggedItemId, Extracted);
            return false;
        }

        EquippedItemId = NAME_None;
    }

    EquippedItemId = DraggedItemId;

    NotifyEquipmentChanged();
    RefreshVisual();

    return true;
}

FReply UKOEquipmentSlotWidget::NativeOnMouseButtonDown(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        if (UnequipItem())
        {
            return FReply::Handled();
        }
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

bool UKOEquipmentSlotWidget::UnequipItem()
{
    ResolveInventoryComponentIfNeeded();

    if (EquippedItemId.IsNone())
    {
        return false;
    }

    UKOInventoryComponent* Inventory = InventoryComponent;
    if (!Inventory)
    {
        return false;
    }

    const int32 Remaining = Inventory->TryAddItem(EKOSlotKind::Item, EquippedItemId, 1);

    if (Remaining > 0)
    {
        return false;
    }

    EquippedItemId = NAME_None;

    NotifyEquipmentChanged();
    RefreshVisual();

    return true;
}

void UKOEquipmentSlotWidget::NotifyEquipmentChanged()
{
    if (SlotType == EKOEquipmentSlotType::Weapon)
    {
        if (AKOPlayerController* PC = Cast<AKOPlayerController>(GetOwningPlayer()))
        {
            PC->OnWeaponCreate.Broadcast();
        }
    }
	
	else
	{
		// 방어력 올리기
	}
}

void UKOEquipmentSlotWidget::RefreshVisual()
{
	const bool bHasItem = !EquippedItemId.IsNone();

	if (EquipmentIconImage)
	{
		if (bHasItem)
		{
			UTexture2D* Icon = UKOItemLibrary::GetIcon(this, EKOSlotKind::Item, EquippedItemId);
			EquipmentIconImage->SetBrushFromTexture(Icon);
			EquipmentIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		else
		{
			EquipmentIconImage->SetBrushFromTexture(nullptr);
			EquipmentIconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	RefreshTooltip();
}

void UKOEquipmentSlotWidget::RefreshTooltip()
{
	if (EquippedItemId.IsNone())
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

	TooltipWidget->SetSlot(EKOSlotKind::Item, EquippedItemId);
	SetToolTip(TooltipWidget);
}