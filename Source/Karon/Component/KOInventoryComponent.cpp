// Copyright Karon Team 5. All Rights Reserved.

#include "Component/KOInventoryComponent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Messaging/KOMessageTypes.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Items/KOItemLibrary.h"
#include "GMRouterSubsystem.h"
#include "StructUtils/InstancedStruct.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UKOInventoryComponent::UKOInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

int32 UKOInventoryComponent::TryAddItem(EKOSlotKind Kind, FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return Count;
    }

    if (!IsItemAccepted(Kind, ItemId))
    {
        return Count;
    }

    const int32 PreviousCount = GetCountOf(ItemId);
    const int32 MaxStack = UKOItemLibrary::GetMaxStack(this, Kind, ItemId);
    int32 Remaining = Count;

    // 1단계: 동일 ItemId 슬롯에 먼저 채운다 
    for (FKOItemSlot& Slot : Slots)
    {
        if (Remaining <= 0)
        {
            break;
        }

        if (Slot.ItemId == ItemId && Slot.Count < MaxStack)
        {
            const int32 Space = MaxStack - Slot.Count;
            const int32 ToAdd = FMath::Min(Space, Remaining);
            Slot.Count += ToAdd;
            Remaining  -= ToAdd;
        }
    }

    // 2단계: 남은 수량을 새 슬롯에 분배한다 
    while (Remaining > 0 && Slots.Num() < MaxSlots)
    {
        const int32 ToAdd = FMath::Min(MaxStack, Remaining);

        FKOItemSlot NewSlot;
        NewSlot.Kind   = Kind;
        NewSlot.ItemId = ItemId;
        NewSlot.Count  = ToAdd;
        Slots.Add(NewSlot);
        Remaining -= ToAdd;
    }

    const int32 NewCount = GetCountOf(ItemId);
    if (NewCount != PreviousCount)
    {
        NotifyInventoryChanged(ItemId, PreviousCount, NewCount);
    }

    return Remaining;
}

bool UKOInventoryComponent::TryRemoveItem(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return false;
    }

    if (!HasEnoughItems(ItemId, Count))
    {
        return false;
    }

    const int32 PreviousCount = GetCountOf(ItemId);
    int32 Remaining = Count;

    for (int32 i = Slots.Num() - 1; i >= 0 && Remaining > 0; --i)
    {
        FKOItemSlot& Slot = Slots[i];
        if (Slot.ItemId != ItemId)
        {
            continue;
        }

        const int32 ToRemove = FMath::Min(Slot.Count, Remaining);
        Slot.Count -= ToRemove;
        Remaining  -= ToRemove;

        if (Slot.Count == 0)
        {
            Slots.RemoveAt(i);
        }
    }

    const int32 NewCount = GetCountOf(ItemId);
    if (NewCount != PreviousCount)
    {
        NotifyInventoryChanged(ItemId, PreviousCount, NewCount);
    }
    return true;
}

int32 UKOInventoryComponent::RemoveAtSlot(int32 SlotIndex, int32 Count)
{
    if (Count <= 0 || !Slots.IsValidIndex(SlotIndex))
    {
        return 0;
    }

    FKOItemSlot& Slot = Slots[SlotIndex];
    if (!Slot.HasItem())
    {
        return 0;
    }

    const FName ItemId       = Slot.ItemId;
    const int32 PreviousTotal = GetCountOf(ItemId);
    const int32 ToRemove      = FMath::Min(Slot.Count, Count);

    Slot.Count -= ToRemove;
    if (Slot.Count == 0)
    {
        Slots.RemoveAt(SlotIndex);
    }

    const int32 NewTotal = GetCountOf(ItemId);
    if (NewTotal != PreviousTotal)
    {
        NotifyInventoryChanged(ItemId, PreviousTotal, NewTotal);
    }
    return ToRemove;
}

bool UKOInventoryComponent::SplitStack(int32 SlotIndex, int32 SplitCount)
{
    if (!Slots.IsValidIndex(SlotIndex))
    {
        return false;
    }

    FKOItemSlot& Source = Slots[SlotIndex];

    if (!Source.HasItem())
    {
        return false;
    }

    if (SplitCount <= 0 || SplitCount >= Source.Count)
    {
        return false;
    }

    if (Slots.Num() >= MaxSlots)
    {
        return false;
    }

    const EKOSlotKind Kind = Source.Kind;
    const FName ItemId     = Source.ItemId;

    Source.Count -= SplitCount;

    FKOItemSlot NewSlot;
    NewSlot.Kind   = Kind;
    NewSlot.ItemId = ItemId;
    NewSlot.Count  = SplitCount;
    Slots.Add(NewSlot);

    const int32 Total = GetCountOf(ItemId);
    NotifyInventoryChanged(ItemId, Total, Total);

    return true;
}

void UKOInventoryComponent::MergeAllStacks()
{
    struct FMergeEntry { EKOSlotKind Kind; FName ItemId; };
    TArray<FMergeEntry> UniqueItems;

    for (const FKOItemSlot& Slot : Slots)
    {
        if (!Slot.HasItem())
        {
            continue;
        }
        const bool bExists = UniqueItems.ContainsByPredicate(
            [&Slot](const FMergeEntry& E){ return E.ItemId == Slot.ItemId; });
        if (!bExists)
        {
            UniqueItems.Add({ Slot.Kind, Slot.ItemId });
        }
    }

    for (const FMergeEntry& Entry : UniqueItems)
    {
        const int32 TotalCount = GetCountOf(Entry.ItemId);
        if (TotalCount <= 0)
        {
            continue;
        }

        if (TryRemoveItem(Entry.ItemId, TotalCount))
        {
            TryAddItem(Entry.Kind, Entry.ItemId, TotalCount);
        }
    }
}

int32 UKOInventoryComponent::GetCountOf(FName ItemId) const
{
    int32 Total = 0;
    for (const FKOItemSlot& Slot : Slots)
    {
        if (Slot.ItemId == ItemId)
        {
            Total += Slot.Count;
        }
    }
    return Total;
}

bool UKOInventoryComponent::HasEnoughItems(FName ItemId, int32 Count) const
{
    return GetCountOf(ItemId) >= Count;
}

const FKOItemSlot* UKOInventoryComponent::GetSlotByIndex(int32 Index) const
{
    if (!Slots.IsValidIndex(Index))
    {
        return nullptr;
    }
    return &Slots[Index];
}

void UKOInventoryComponent::NotifyInventoryChanged(FName ItemId, int32 PreviousCount, int32 NewCount)
{
    FKOInventoryChangedMessage Msg;
    Msg.ItemId        = ItemId;
    Msg.PreviousCount = PreviousCount;
    Msg.NewCount      = NewCount;

    // GMRouter GMS 브로드캐스트
    const UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    UGameInstance* GI = World->GetGameInstance();
    if (!GI)
    {
        return;
    }

    if (UGMRouterSubsystem* GMS = GI->GetSubsystem<UGMRouterSubsystem>())
    {
        GMS->BroadcastMessage(
            KOGameplayTags::Data_Message_Inventory_Changed,
            FInstancedStruct::Make(Msg)
        );
    }
}

bool UKOInventoryComponent::IsItemAccepted(EKOSlotKind Kind, FName ItemId) const
{
    // DataTable에 존재하지 않으면 무조건 거부
    if (!UKOItemLibrary::HasRow(this, Kind, ItemId))
    {
        return false;
    }

    switch (Kind)
    {
    case EKOSlotKind::Item:
    {
        if (AcceptedItemsQuery.IsEmpty())
        {
            return true;
        }
        const FKOItemRow* Row = UKOItemLibrary::GetItemRow(this, ItemId);
        return Row && AcceptedItemsQuery.Matches(Row->Categories);
    }
    case EKOSlotKind::Factory:
        return bAcceptFactories;
    }
    return false;
}
