// Copyright Karon Team 5. All Rights Reserved.

#include "Component/KOStorageInventoryComponent.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UKOStorageInventoryComponent::UKOStorageInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UKOStorageInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // ExtraSlots 합산 — BeginPlay 시점에 한 번만 수행
    MaxSlots += ExtraSlots;
}

bool UKOStorageInventoryComponent::CanAcceptItem(FGameplayTag ItemTag) const
{
    // AllowedCategories가 비어 있으면 모두 허용
    if (AllowedCategories.IsEmpty())
    {
        return true;
    }

    const UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    const UGameInstance* GI = World->GetGameInstance();
    if (!GI)
    {
        return false;
    }

    const UKOLoadSubsystem* LoadSub = GI->GetSubsystem<UKOLoadSubsystem>();
    if (!LoadSub)
    {
        return false;
    }

    const FKOItemRow* Row = LoadSub->FindItemRow(ItemTag);
    if (!Row)
    {
        return false;
    }

    // 아이템의 Categories 중 허용 목록과 겹치는 태그가 하나라도 있으면 허용
    return Row->Categories.HasAny(AllowedCategories);
}

int32 UKOStorageInventoryComponent::TryAddItem(FGameplayTag ItemTag, int32 Count)
{
    if (!CanAcceptItem(ItemTag))
    {
        return Count;
    }

    const int32 Remaining = Super::TryAddItem(ItemTag, Count);

    if (bAutoSortOnChange && Remaining < Count)
    {
        SortSlots();
    }

    return Remaining;
}

bool UKOStorageInventoryComponent::IsItemAccepted(FGameplayTag ItemTag) const
{
    // 부모의 AcceptedItemsQuery 검사 먼저
    if (!Super::IsItemAccepted(ItemTag))
    {
        return false;
    }

    return CanAcceptItem(ItemTag);
}

void UKOStorageInventoryComponent::SortSlots()
{
    Slots.Sort([](const FKOItemSlot& A, const FKOItemSlot& B)
    {
        return A.ItemTag.GetTagName().LexicalLess(B.ItemTag.GetTagName());
    });
}
