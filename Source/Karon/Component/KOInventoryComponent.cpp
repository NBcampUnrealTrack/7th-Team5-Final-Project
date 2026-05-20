// Copyright Karon Team 5. All Rights Reserved.

#include "Component/KOInventoryComponent.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "GMRouterSubsystem.h"
#include "StructUtils/InstancedStruct.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UKOInventoryComponent::UKOInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

int32 UKOInventoryComponent::TryAddItem(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return Count;
    }

    if (!IsItemAccepted(ItemId))
    {
        return Count;
    }

    const int32 MaxStack = GetMaxStackForItem(ItemId);
    int32 Remaining = Count;

    // 1단계: 동일 아이템이 있는 기존 슬롯에 먼저 채운다
    for (FKOItemSlot& Slot : Slots)
    {
        if (Remaining <= 0)
        {
            break;
        }

        if (Slot.ItemId == ItemId && Slot.Count < MaxStack)
        {
            const int32 PreviousCount = GetCountOf(ItemId);
            const int32 Space = MaxStack - Slot.Count;
            const int32 ToAdd = FMath::Min(Space, Remaining);
            Slot.Count += ToAdd;
            Remaining  -= ToAdd;

            NotifyChanged(ItemId, PreviousCount, GetCountOf(ItemId));
        }
    }

    // 2단계: 남은 수량을 새 슬롯에 분배한다
    while (Remaining > 0 && Slots.Num() < MaxSlots)
    {
        const int32 PreviousCount = GetCountOf(ItemId);
        const int32 ToAdd = FMath::Min(MaxStack, Remaining);

        FKOItemSlot NewSlot;
        NewSlot.ItemId = ItemId;
        NewSlot.Count  = ToAdd;
        Slots.Add(NewSlot);
        Remaining -= ToAdd;

        NotifyChanged(ItemId, PreviousCount, GetCountOf(ItemId));
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

    NotifyChanged(ItemId, PreviousCount, GetCountOf(ItemId));
    return true;
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

void UKOInventoryComponent::NotifyChanged(FName ItemId, int32 PreviousCount, int32 NewCount)
{
    FKOInventoryChangedMessage Msg;
    Msg.ItemId        = ItemId;
    Msg.PreviousCount = PreviousCount;
    Msg.NewCount      = NewCount;

    // 1. 네이티브 델리게이트 브로드캐스트
    OnInventoryChangedNative.Broadcast(Msg);

    // 2. GMRouter GMS 브로드캐스트
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

bool UKOInventoryComponent::IsItemAccepted(FName ItemId) const
{
    // 쿼리가 비어 있으면 모두 허용
    if (!AcceptedItemsQuery.IsEmpty())
    {
        const UWorld* World = GetWorld();
        if (World)
        {
            if (const UGameInstance* GI = World->GetGameInstance())
            {
                if (const UKOLoadSubsystem* LoadSub = GI->GetSubsystem<UKOLoadSubsystem>())
                {
                    if (const FKOItemRow* Row = LoadSub->FindItemRow(ItemId))
                    {
                        return AcceptedItemsQuery.Matches(Row->Categories);
                    }
                }
            }
        }
        // DataTable에 없는 아이템이라면 거부
        return false;
    }
    return true;
}

int32 UKOInventoryComponent::GetMaxStackForItem(FName ItemId) const
{
    constexpr int32 DefaultMaxStack = 100;

    const UWorld* World = GetWorld();
    if (!World)
    {
        return DefaultMaxStack;
    }

    const UGameInstance* GI = World->GetGameInstance();
    if (!GI)
    {
        return DefaultMaxStack;
    }

    const UKOLoadSubsystem* LoadSub = GI->GetSubsystem<UKOLoadSubsystem>();
    if (!LoadSub)
    {
        return DefaultMaxStack;
    }

    const FKOItemRow* Row = LoadSub->FindItemRow(ItemId);
    return Row ? Row->MaxStack : DefaultMaxStack;
}
