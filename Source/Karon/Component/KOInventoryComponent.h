// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Items/KOItemSlot.h"
#include "KOInventoryComponent.generated.h"

class UGMRouterSubsystem;

UCLASS(ClassGroup = "KO|Inventory")
class KARON_API UKOInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UKOInventoryComponent();

    UPROPERTY(EditAnywhere, Category = "KO|Inventory")
    int32 MaxSlots = 40;
    
    UPROPERTY(EditDefaultsOnly, Category = "KO|Inventory")
    FGameplayTagQuery AcceptedItemsQuery;
    
    UPROPERTY(EditDefaultsOnly, Category = "KO|Inventory")
    bool bAcceptFactories = true;

    // 아이템 조작 API
    virtual int32 TryAddItem(EKOSlotKind Kind, FName ItemId, int32 Count);
    virtual bool  TryRemoveItem(FName ItemId, int32 Count);

    /** 슬롯 인덱스 기반 차감. 실제 차감된 수량 반환. */
    int32 RemoveAtSlot(int32 SlotIndex, int32 Count);

    /** 두 슬롯 인덱스 스왑(또는 같은 ItemId면 머지). 두 인덱스 모두 현재 Slots 범위 내여야 함. */
    bool  SwapSlots(int32 IndexA, int32 IndexB);

    bool SplitStack(int32 SlotIndex, int32 SplitCount);
    void MergeAllStacks();

    // 아이템 조회 API
    int32 GetCountOf(FName ItemId) const;
    bool  HasEnoughItems(FName ItemId, int32 Count) const;

    const TArray<FKOItemSlot>& GetSlots() const { return Slots; }

    /** 슬롯 인덱스로 안전 조회. 유효하지 않으면 nullptr 반환. */
    const FKOItemSlot* GetSlotByIndex(int32 Index) const;

protected:
    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;

    /** Slots를 MaxSlots 크기로 패딩(빈 슬롯으로). 이미 충분하면 no-op. */
    void EnsureSlotsCapacity();

    /** 첫 빈 슬롯 인덱스. 없으면 INDEX_NONE. */
    int32 FindFirstEmptySlot() const;

    UPROPERTY()
    TArray<FKOItemSlot> Slots;

    void NotifyInventoryChanged(FName ItemId, int32 PreviousCount, int32 NewCount);
    virtual bool IsItemAccepted(EKOSlotKind Kind, FName ItemId) const;
};
