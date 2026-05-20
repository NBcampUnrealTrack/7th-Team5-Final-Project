// Copyright Karon Team 5. All Rights Reserved.

#include "Subsystem/KOItemManagerSubsystem.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Component/KOInventoryComponent.h"
#include "Items/KOItemTypes.h"

void UKOItemManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    // UKOLoadSubsystem이 먼저 초기화되도록 의존성을 선언한다
    Collection.InitializeDependency(UKOLoadSubsystem::StaticClass());
}

void UKOItemManagerSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

int32 UKOItemManagerSubsystem::GetMaxStack(FName ItemId) const
{
    constexpr int32 DefaultMaxStack = 100;

    if (ItemId.IsNone())
    {
        return DefaultMaxStack;
    }

    const UKOLoadSubsystem* LoadSub = GetGameInstance()->GetSubsystem<UKOLoadSubsystem>();
    if (!LoadSub)
    {
        return DefaultMaxStack;
    }

    const FKOItemRow* Row = LoadSub->FindItemRow(ItemId);
    return Row ? Row->MaxStack : DefaultMaxStack;
}

int32 UKOItemManagerSubsystem::TransferItems(
    UKOInventoryComponent* From,
    UKOInventoryComponent* To,
    FName                  ItemId,
    int32                  RequestedCount
)
{
    if (!From || !To || ItemId.IsNone() || RequestedCount <= 0)
    {
        return 0;
    }

    // From에서 실제로 꺼낼 수 있는 수량
    const int32 Available = From->GetCountOf(ItemId);
    const int32 ToTransfer = FMath::Min(Available, RequestedCount);

    if (ToTransfer <= 0)
    {
        return 0;
    }

    // To에 추가 시도 — 잔여(못 넣은 수량) 계산
    const int32 FailedToAdd = To->TryAddItem(ItemId, ToTransfer);
    const int32 ActuallyAdded = ToTransfer - FailedToAdd;

    if (ActuallyAdded > 0)
    {
        // From에서 실제로 추가된 만큼 제거
        From->TryRemoveItem(ItemId, ActuallyAdded);
    }

    return ActuallyAdded;
}

bool UKOItemManagerSubsystem::SplitStack(UKOInventoryComponent* Inventory, int32 SlotIndex, int32 SplitCount)
{
    if (!Inventory)
    {
        return false;
    }

    const TArray<FKOItemSlot>& Slots = Inventory->GetSlots();

    if (!Slots.IsValidIndex(SlotIndex))
    {
        return false;
    }

    const FKOItemSlot& TargetSlot = Slots[SlotIndex];

    if (!TargetSlot.IsValid())
    {
        return false;
    }

    // SplitCount는 1 이상, 슬롯 수량 미만이어야 함
    if (SplitCount <= 0 || SplitCount >= TargetSlot.Count)
    {
        return false;
    }

    const FName ItemId = TargetSlot.ItemId;

    // 원래 슬롯에서 SplitCount만큼 제거
    if (!Inventory->TryRemoveItem(ItemId, SplitCount))
    {
        return false;
    }

    // 새 슬롯에 추가. 실패(슬롯 부족)하면 롤백하여 원복
    const int32 Remaining = Inventory->TryAddItem(ItemId, SplitCount);
    if (Remaining > 0)
    {
        // 추가 실패한 수량을 다시 복원
        Inventory->TryAddItem(ItemId, Remaining);
        return false;
    }

    return true;
}

void UKOItemManagerSubsystem::MergeAllStacks(UKOInventoryComponent* Inventory)
{
    if (!Inventory)
    {
        return;
    }

    // 현재 보유 중인 모든 고유 아이템 식별자 수집
    TArray<FName> UniqueItems;
    for (const FKOItemSlot& Slot : Inventory->GetSlots())
    {
        if (Slot.IsValid())
        {
            UniqueItems.AddUnique(Slot.ItemId);
        }
    }

    for (const FName& ItemId : UniqueItems)
    {
        const int32 TotalCount = Inventory->GetCountOf(ItemId);
        if (TotalCount <= 0)
        {
            continue;
        }

        // 전부 제거 후 재추가하면 TryAddItem 내부 로직이 스택을 자동으로 병합한다
        if (Inventory->TryRemoveItem(ItemId, TotalCount))
        {
            Inventory->TryAddItem(ItemId, TotalCount);
        }
    }
}
