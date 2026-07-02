#include "KOEquipmentSlotWidget.h"

#include "Blueprint/DragDropOperation.h"
#include "Components/Image.h"
#include "InputCoreTypes.h"

#include "Component/Inventory/KOInventoryComponent.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Data/KODataTableTypes.h"
#include "Data/Equipment/KOWeaponDefinition.h"
#include "UI/Inventory/KOItemDragDropOperation.h"
#include "UI/Inventory/KOItemDragSource.h"
#include "UI/ItemTooltip/KOItemTooltipWidget.h"
#include "Kismet/GameplayStatics.h"

void UKOEquipmentSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ResolveInventoryComponentIfNeeded();
	SyncFromEquipmentComponent();
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

UKOEquipmentComponent* UKOEquipmentSlotWidget::ResolveEquipmentComponent() const
{
	APlayerController* PC = nullptr;

	if (UWorld* World = GetWorld())
	{
		PC = UGameplayStatics::GetPlayerController(World, 0);
	}

	if (!PC)
	{
		PC = GetOwningPlayer();
	}

	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EquipmentSlotUI] ResolveEquipmentComponent 실패: PC NULL"));
		return nullptr;
	}

	APawn* Pawn = PC->GetPawn();
	if (!Pawn)
	{
		UE_LOG(LogTemp, Warning, TEXT("[EquipmentSlotUI] ResolveEquipmentComponent 실패: Pawn NULL"));
		return nullptr;
	}

	UKOEquipmentComponent* EquipmentComponent = Pawn->FindComponentByClass<UKOEquipmentComponent>();

	return EquipmentComponent;
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

	if (EquipmentRow->SlotType != SlotType)
	{
		return false;
	}

	if (SlotType == EKOEquipmentSlotType::Weapon)
	{
		if (EquipmentRow->WeaponDefinition.IsNull())
		{
			return false;
		}
	}

	return true;
}

void UKOEquipmentSlotWidget::SyncFromEquipmentComponent()
{
	UKOEquipmentComponent* EquipmentComponent = ResolveEquipmentComponent();
	if (!EquipmentComponent)
	{
		EquippedItemId = NAME_None;
		RefreshVisual();
		return;
	}

	if (SlotType == EKOEquipmentSlotType::Weapon)
	{
		EquippedItemId = EquipmentComponent->GetCurrentWeaponItemId();
	}
	
	else
	{
		EquippedItemId = EquipmentComponent->GetEquippedArmorItemId(SlotType);
	}

	RefreshVisual();
}


bool UKOEquipmentSlotWidget::ApplyEquipmentToComponent()
{
	UKOEquipmentComponent* EquipmentComponent = ResolveEquipmentComponent();
	if (!EquipmentComponent)
	{
		return false;
	}

	// 무기 슬롯
	if (SlotType == EKOEquipmentSlotType::Weapon)
	{
		if (EquippedItemId.IsNone())
		{
			EquipmentComponent->UnequipWeapon();
			return true;
		}

		UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
		if (!LoadSub)
		{
			return false;
		}

		UKOWeaponDefinition* WeaponDef = LoadSub->ResolveWeaponDefinitionByItemId(EquippedItemId);

		if (!WeaponDef)
		{
			return false;
		}

		const bool bEquipped = EquipmentComponent->EquipWeaponFromItem(EquippedItemId, WeaponDef);

		if (!bEquipped)
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("[EquipmentSlot] EquipWeaponFromItem 실패: ItemId=%s, WeaponDef=%s"),
				*EquippedItemId.ToString(),
				*GetNameSafe(WeaponDef)
			);

			return false;
		}

		return true;
	}
	
	// 방어구 슬롯
	if (EquippedItemId.IsNone())
	{
		EquipmentComponent->UnequipArmor(SlotType);
		return true;
	}

	return EquipmentComponent->EquipArmorFromItem(SlotType, EquippedItemId);
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
	
	const FName PreviousEquippedItemId = EquippedItemId;

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

	if (!ApplyEquipmentToComponent())
	{
		EquippedItemId = PreviousEquippedItemId;
		ItemDragOperation->Source->Restore(DraggedItemId, Extracted);
		return false;
	}
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

	ApplyEquipmentToComponent();
	RefreshVisual();

	return true;
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