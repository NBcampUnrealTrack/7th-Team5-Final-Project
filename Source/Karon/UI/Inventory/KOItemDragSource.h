// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "KOItemDragSource.generated.h"

class UKOInventoryComponent;
class UKOFactoryProcessorComponent;
class UKOEnergyProducerComponent;
class UKOBuildUIComponent;
class UKOEquipmentSlotWidget;

/**
 * 드래그가 시작된 출발지를 추상화. 드롭 핸들러는 종류를 알 필요 없이
 * Extract → (실패 시) Restore 만 호출하면 된다.
 *
 * 인스턴스 수명: 한 번의 드래그 동안. Drag 시작 시 NewObject<>(DragOp) 로 생성,
 * DragOp가 GC되면 함께 해제. 컴포넌트 참조는 weak.
 */
UCLASS(Abstract)
class KARON_API UKOItemDragSource : public UObject
{
    GENERATED_BODY()

public:
    /** 출발지에서 ItemId를 Count만큼 차감 시도. 실제 차감된 수량 반환. */
    virtual int32 Extract(FName ItemId, int32 Count) { return 0; }

    /** Extract 후 도착지가 거부한 잔량을 출발지로 되돌림. */
    virtual void  Restore(FName ItemId, int32 Count) {}

    /** 인벤토리 슬롯에서 출발했는지. 슬롯 단위 스왑/식별용. */
    virtual bool  IsInventorySource() const { return false; }
};

/** 인벤토리 특정 슬롯에서 출발. */
UCLASS()
class KARON_API UKOInventorySlotItemSource : public UKOItemDragSource
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TWeakObjectPtr<UKOInventoryComponent> Inventory;

    int32 SlotIndex = INDEX_NONE;

    virtual int32 Extract(FName ItemId, int32 Count) override;
    virtual void  Restore(FName ItemId, int32 Count) override;
    virtual bool  IsInventorySource() const override { return true; }

    UKOInventoryComponent* GetInventory() const { return Inventory.Get(); }
};

/** 장비 슬롯에서 출발 (드래그로 장착 해제). */
UCLASS()
class KARON_API UKOEquipmentSlotItemSource : public UKOItemDragSource
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TWeakObjectPtr<UKOEquipmentSlotWidget> EquipmentSlot;

    virtual int32 Extract(FName ItemId, int32 Count) override;
    virtual void  Restore(FName ItemId, int32 Count) override;
};

/** Processor의 InputBuffer에서 출발. */
UCLASS()
class KARON_API UKOProcessorInputItemSource : public UKOItemDragSource
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TWeakObjectPtr<UKOFactoryProcessorComponent> Processor;

    virtual int32 Extract(FName ItemId, int32 Count) override;
    virtual void  Restore(FName ItemId, int32 Count) override;
};

/** Processor의 OutputBuffer에서 출발. */
UCLASS()
class KARON_API UKOProcessorOutputItemSource : public UKOItemDragSource
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TWeakObjectPtr<UKOFactoryProcessorComponent> Processor;

    virtual int32 Extract(FName ItemId, int32 Count) override;
    virtual void  Restore(FName ItemId, int32 Count) override;
};

/** Build QuickSlot 칸에서 출발. 인벤토리처럼 보관소가 아니므로 Extract/Restore는 no-op. */
UCLASS()
class KARON_API UKOBuildQuickSlotItemSource : public UKOItemDragSource
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TWeakObjectPtr<UKOBuildUIComponent> BuildUI;

    int32 SlotIndex = INDEX_NONE;

    UKOBuildUIComponent* GetBuildUI() const { return BuildUI.Get(); }
};

/** Producer의 FuelBuffer에서 출발. */
UCLASS()
class KARON_API UKOProducerFuelItemSource : public UKOItemDragSource
{
    GENERATED_BODY()

public:
    UPROPERTY()
    TWeakObjectPtr<UKOEnergyProducerComponent> Producer;

    virtual int32 Extract(FName ItemId, int32 Count) override;
    virtual void  Restore(FName ItemId, int32 Count) override;
};
