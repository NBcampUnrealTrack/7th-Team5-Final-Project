#include "KOEquipmentSlotWidget.h"

#include "Blueprint/DragDropOperation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
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
#include "Subsystem/KOQuestGuideSubsystem.h"

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

bool UKOEquipmentSlotWidget::NotifyIfBlockedBySkill(UKOEquipmentComponent* EquipmentComponent) const
{
    if (EquipmentComponent && EquipmentComponent->CanEquip())
    {
        return false;
    }

    if (EquipmentComponent->OnEquipmentChangeBlocked.IsBound())
    {
        EquipmentComponent->OnEquipmentChangeBlocked.Broadcast();
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
			return EquipmentComponent->UnequipWeapon();
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
		return EquipmentComponent->UnequipArmor(SlotType);
	}

	return EquipmentComponent->EquipArmorFromItem(SlotType, EquippedItemId);
}

bool UKOEquipmentSlotWidget::NativeOnDrop(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    ResolveInventoryComponentIfNeeded();

    if (!InventoryComponent)
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

    return TryEquipFromSource(DraggedItemId, ItemDragOperation->Source);
}

bool UKOEquipmentSlotWidget::TryEquipFromSource(FName ItemId, UKOItemDragSource* Source)
{
    ResolveInventoryComponentIfNeeded();

    UKOInventoryComponent* Inventory = InventoryComponent;
    if (!Inventory || !Source || ItemId.IsNone())
    {
        return false;
    }

    if (!CanAcceptItem(ItemId))
    {
        return false;
    }

    UKOEquipmentComponent* EquipmentComponent = ResolveEquipmentComponent();
    if (NotifyIfBlockedBySkill(EquipmentComponent))
    {
        return false;
    }

    const int32 Extracted = Source->Extract(ItemId, 1);
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
            Source->Restore(ItemId, Extracted);
            return false;
        }

        EquippedItemId = NAME_None;
    }

    EquippedItemId = ItemId;

    if (!ApplyEquipmentToComponent())
    {
        EquippedItemId = PreviousEquippedItemId;
        Source->Restore(ItemId, Extracted);
        return false;
    }
    RefreshVisual();

    // 퀘스트
    if (SlotType == EKOEquipmentSlotType::Weapon)
    {
        if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
        {
            QuestGuide->NotifyWeaponEquipped(EquippedItemId);
        }
    }

    return true;
}

bool UKOEquipmentSlotWidget::TryEquipItemFromInventorySlot(int32 InventorySlotIndex, FName ItemId)
{
    ResolveInventoryComponentIfNeeded();

    if (!InventoryComponent || ItemId.IsNone())
    {
        return false;
    }

    UKOInventorySlotItemSource* Source = NewObject<UKOInventorySlotItemSource>(this);
    Source->Inventory  = InventoryComponent;
    Source->SlotIndex  = InventorySlotIndex;

    return TryEquipFromSource(ItemId, Source);
}

int32 UKOEquipmentSlotWidget::ExtractEquippedItem(FName ItemId, int32 Count)
{
    if (EquippedItemId.IsNone() || EquippedItemId != ItemId || Count <= 0)
    {
        return 0;
    }

    if (NotifyIfBlockedBySkill(ResolveEquipmentComponent()))
    {
        return 0;
    }

    const FName PreviousEquippedItemId = EquippedItemId;
    EquippedItemId = NAME_None;

    if (!ApplyEquipmentToComponent())
    {
        EquippedItemId = PreviousEquippedItemId;
        return 0;
    }

    RefreshVisual();
    return 1;
}

void UKOEquipmentSlotWidget::RestoreEquippedItem(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return;
    }

    EquippedItemId = ItemId;

    if (!ApplyEquipmentToComponent())
    {
        EquippedItemId = NAME_None;
        return;
    }

    RefreshVisual();
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
    else if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && !EquippedItemId.IsNone())
    {
        FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(
            InMouseEvent,
            this,
            EKeys::LeftMouseButton
        );

        return Reply.NativeReply;
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UKOEquipmentSlotWidget::NativeOnDragDetected(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent,
    UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    if (EquippedItemId.IsNone())
    {
        return;
    }

    FKOItemSlot DragSlot;
    DragSlot.Kind    = EKOSlotKind::Item;
    DragSlot.ItemId  = EquippedItemId;
    DragSlot.Count   = 1;

    const FText ItemDisplayName = UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Item, EquippedItemId);
    UTexture2D* ItemIcon        = UKOItemLibrary::GetIcon(this, EKOSlotKind::Item, EquippedItemId);

    UKOItemDragDropOperation* DragOp = UKOItemDragDropOperation::CreateItemDragOperation(
        this,
        DragSlot,
        ItemDisplayName,
        ItemIcon,
        DragVisualSize,
        DragVisualOpacity,
        nullptr
    );

    if (DragOp)
    {
        UKOEquipmentSlotItemSource* Src = NewObject<UKOEquipmentSlotItemSource>(DragOp);
        Src->EquipmentSlot = this;
        DragOp->Source = Src;
    }

    OutOperation = DragOp;
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

    if (NotifyIfBlockedBySkill(ResolveEquipmentComponent()))
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
