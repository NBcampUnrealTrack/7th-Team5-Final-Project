// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Inventory/KOItemDragSource.h"

#include "Component/Factory/KOEnergyProducerComponent.h"
#include "Component/Factory/KOFactoryProcessorComponent.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Items/KOItemSlot.h"
#include "UI/Inventory/KOEquipmentSlotWidget.h"

int32 UKOInventorySlotItemSource::Extract(FName ItemId, int32 Count)
{
    UKOInventoryComponent* Inv = Inventory.Get();
    if (!Inv)
    {
        return 0;
    }
    return Inv->RemoveAtSlot(SlotIndex, Count);
}

void UKOInventorySlotItemSource::Restore(FName ItemId, int32 Count)
{
    UKOInventoryComponent* Inv = Inventory.Get();
    if (!Inv || ItemId.IsNone() || Count <= 0)
    {
        return;
    }
    // 정확한 슬롯 복원 API가 없으므로 TryAddItem에 위임 (앞쪽 빈 슬롯에 들어감).
    Inv->TryAddItem(EKOSlotKind::Item, ItemId, Count);
}

int32 UKOEquipmentSlotItemSource::Extract(FName ItemId, int32 Count)
{
    UKOEquipmentSlotWidget* Slot = EquipmentSlot.Get();
    return Slot ? Slot->ExtractEquippedItem(ItemId, Count) : 0;
}

void UKOEquipmentSlotItemSource::Restore(FName ItemId, int32 Count)
{
    if (UKOEquipmentSlotWidget* Slot = EquipmentSlot.Get())
    {
        Slot->RestoreEquippedItem(ItemId, Count);
    }
}

int32 UKOProcessorInputItemSource::Extract(FName ItemId, int32 Count)
{
    UKOFactoryProcessorComponent* Proc = Processor.Get();
    return Proc ? Proc->TryExtractInputItem(ItemId, Count) : 0;
}

void UKOProcessorInputItemSource::Restore(FName ItemId, int32 Count)
{
    if (UKOFactoryProcessorComponent* Proc = Processor.Get())
    {
        Proc->RestoreInputBuffer(ItemId, Count);
    }
}

int32 UKOProcessorOutputItemSource::Extract(FName ItemId, int32 Count)
{
    UKOFactoryProcessorComponent* Proc = Processor.Get();
    return Proc ? Proc->TryExtractItem(ItemId, Count) : 0;
}

void UKOProcessorOutputItemSource::Restore(FName ItemId, int32 Count)
{
    if (UKOFactoryProcessorComponent* Proc = Processor.Get())
    {
        Proc->RestoreOutputBuffer(ItemId, Count);
    }
}

int32 UKOProducerFuelItemSource::Extract(FName ItemId, int32 Count)
{
    UKOEnergyProducerComponent* Prod = Producer.Get();
    return Prod ? Prod->TryExtractFuel(Count) : 0;
}

void UKOProducerFuelItemSource::Restore(FName ItemId, int32 Count)
{
    if (UKOEnergyProducerComponent* Prod = Producer.Get())
    {
        Prod->RestoreFuelBuffer(ItemId, Count);
    }
}
