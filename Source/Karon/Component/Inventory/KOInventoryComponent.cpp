// Copyright Karon Team 5. All Rights Reserved.

#include "Component/Inventory/KOInventoryComponent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Items/KOItemLibrary.h"
#include "GMRouterSubsystem.h"
#include "StructUtils/InstancedStruct.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Utility/Messaging/KOMessageTypes.h"

UKOInventoryComponent::UKOInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bWantsInitializeComponent = true;
}

void UKOInventoryComponent::InitializeComponent()
{
    Super::InitializeComponent();
    EnsureSlotsCapacity();
}

void UKOInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    EnsureSlotsCapacity();
}

void UKOInventoryComponent::EnsureSlotsCapacity()
{
    if (MaxSlots <= 0)
    {
        return;
    }
    if (Slots.Num() < MaxSlots)
    {
        Slots.SetNum(MaxSlots);
    }
}

int32 UKOInventoryComponent::FindFirstEmptySlot() const
{
    for (int32 i = 0; i < Slots.Num(); ++i)
    {
        if (!Slots[i].HasItem())
        {
            return i;
        }
    }
    return INDEX_NONE;
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
    if (MaxStack <= 0)
    {
        return Count;
    }
    
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

    // 2단계: 남은 수량을 빈 슬롯에 분배한다
    while (Remaining > 0)
    {
        const int32 EmptyIdx = FindFirstEmptySlot();
        if (EmptyIdx == INDEX_NONE)
        {
            break;
        }
        const int32 ToAdd = FMath::Min(MaxStack, Remaining);

        FKOItemSlot& NewSlot = Slots[EmptyIdx];
        NewSlot.Kind   = Kind;
        NewSlot.ItemId = ItemId;
        NewSlot.Count  = ToAdd;
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
    
    if (!RemoveItemFromSlots(Slots, ItemId, Count))
    {
        return false;
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
        Slot = FKOItemSlot{};
    }

    const int32 NewTotal = GetCountOf(ItemId);
    if (NewTotal != PreviousTotal)
    {
        NotifyInventoryChanged(ItemId, PreviousTotal, NewTotal);
    }
    return ToRemove;
}

bool UKOInventoryComponent::SwapSlots(int32 IndexA, int32 IndexB)
{
    if (IndexA == IndexB) return false;
    if (!Slots.IsValidIndex(IndexA) || !Slots.IsValidIndex(IndexB)) return false;

    const FKOItemSlot SnapshotA = Slots[IndexA];
    const FKOItemSlot SnapshotB = Slots[IndexB];

    const bool bSameItem = SnapshotA.HasItem() && SnapshotB.HasItem()
        && SnapshotA.Kind == SnapshotB.Kind
        && SnapshotA.ItemId == SnapshotB.ItemId;

    if (bSameItem)
    {
        const int32 MaxStack = UKOItemLibrary::GetMaxStack(this, SnapshotA.Kind, SnapshotA.ItemId);
        const int32 Space    = FMath::Max(0, MaxStack - SnapshotB.Count);
        const int32 Moved    = FMath::Min(Space, SnapshotA.Count);
        if (Moved <= 0)
        {
            return false;
        }
        Slots[IndexB].Count += Moved;
        Slots[IndexA].Count -= Moved;
        if (Slots[IndexA].Count <= 0)
        {
            Slots[IndexA] = FKOItemSlot{};
        }
    }
    else
    {
        Swap(Slots[IndexA], Slots[IndexB]);
    }

    // 카운트는 변동 없음(머지면 동일 ItemId 내 분포만 이동) — 메시지로 UI 리빌드만 트리거.
    const FName NotifyItemId = !SnapshotA.ItemId.IsNone() ? SnapshotA.ItemId : SnapshotB.ItemId;
    NotifyInventoryChanged(NotifyItemId, GetCountOf(NotifyItemId), GetCountOf(NotifyItemId));
    return true;
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

    const int32 EmptyIdx = FindFirstEmptySlot();
    if (EmptyIdx == INDEX_NONE)
    {
        return false;
    }

    const EKOSlotKind Kind = Source.Kind;
    const FName ItemId     = Source.ItemId;

    Source.Count -= SplitCount;

    FKOItemSlot& NewSlot = Slots[EmptyIdx];
    NewSlot.Kind   = Kind;
    NewSlot.ItemId = ItemId;
    NewSlot.Count  = SplitCount;

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
    return GetCountOfInSlots(Slots, ItemId);
}

int32 UKOInventoryComponent::GetCountOfInSlots(const TArray<FKOItemSlot>& SourceSlots, FName ItemId) const
{
    int32 Total = 0;

    for (const FKOItemSlot& Slot : SourceSlots)
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

bool UKOInventoryComponent::RemoveItemFromSlots(TArray<FKOItemSlot>& TargetSlots, FName ItemId, int32 Count) const
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return false;
    }

    if (GetCountOfInSlots(TargetSlots, ItemId) < Count)
    {
        return false;
    }

    int32 Remaining = Count;

    for (int32 i = TargetSlots.Num() - 1; i >= 0 && Remaining > 0; --i)
    {
        FKOItemSlot& Slot = TargetSlots[i];

        if (Slot.ItemId != ItemId)
        {
            continue;
        }

        const int32 ToRemove = FMath::Min(Slot.Count, Remaining);
        Slot.Count -= ToRemove;
        Remaining -= ToRemove;

        if (Slot.Count <= 0)
        {
            Slot = FKOItemSlot{};
        }
    }

    return Remaining <= 0;
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

int32 UKOInventoryComponent::GetAddCountAfterRemoving(EKOSlotKind AddKind, FName AddItemId, int32 AddCount,
    const TArray<TPair<FName, int32>>& ItemsToRemove) const
{
    if (AddItemId.IsNone() || AddCount <= 0)
    {
        return 0;
    }

    TArray<FKOItemSlot> SimulatedSlots = Slots;

    for (const TPair<FName, int32>& RemoveItem : ItemsToRemove)
    {
        const FName RemoveItemId = RemoveItem.Key;
        int32 RemoveCount = RemoveItem.Value;

        if (RemoveItemId.IsNone() || RemoveCount <= 0)
        {
            continue;
        }

        if (!RemoveItemFromSlots(SimulatedSlots, RemoveItemId, RemoveCount))
        {
            return 0;
        }
    }

    return GetAddCountInSlots(SimulatedSlots, AddKind, AddItemId, AddCount);
}

int32 UKOInventoryComponent::GetAddCountInSlots(const TArray<FKOItemSlot>& SourceSlots, EKOSlotKind Kind,
    FName ItemId, int32 Count) const
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return 0;
    }

    if (!IsItemAccepted(Kind, ItemId))
    {
        return 0;
    }

    const int32 MaxStack = UKOItemLibrary::GetMaxStack(this, Kind, ItemId);
    if (MaxStack <= 0)
    {
        return 0;
    }

    int32 AddableCount = 0;

    // 같은 아이템이 들어 있는 기존 슬롯의 남은 공간 계산
    for (const FKOItemSlot& Slot : SourceSlots)
    {
        if (!Slot.HasItem())
        {
            continue;
        }

        if (Slot.ItemId == ItemId && Slot.Count < MaxStack)
        {
            AddableCount += MaxStack - Slot.Count;

            if (AddableCount >= Count)
            {
                return Count;
            }
        }
    }

    // 빈 슬롯에 들어갈 수 있는 수량 계산
    for (const FKOItemSlot& Slot : SourceSlots)
    {
        if (Slot.HasItem())
        {
            continue;
        }

        AddableCount += MaxStack;

        if (AddableCount >= Count)
        {
            return Count;
        }
    }

    return FMath::Min(AddableCount, Count);
}
