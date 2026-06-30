#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/KODataTableTypes.h"
#include "KOEquipmentSlotWidget.generated.h"

class UImage;
class UDragDropOperation;
class UKOInventoryComponent;
class UKOItemTooltipWidget;
class UKOEquipmentComponent;

UCLASS()
class KARON_API UKOEquipmentSlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetInventoryComponent(UKOInventoryComponent* InInventory);

    /** 장착 해제: 장착 무기를 인벤토리로 되돌림 */
    bool UnequipItem();

    FName GetEquippedItemId() const { return EquippedItemId; }
    
    /** EquipmentComponent 상태를 UI 슬롯에 반영 */
    UFUNCTION(BlueprintCallable, Category = "KO|EquipmentSlot")
    void SyncFromEquipmentComponent();

protected:
    virtual void NativeConstruct() override;

    virtual bool NativeOnDrop(
        const FGeometry& InGeometry,
        const FDragDropEvent& InDragDropEvent,
        UDragDropOperation* InOperation
    ) override;

    virtual FReply NativeOnMouseButtonDown(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent
    ) override;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KO|EquipmentSlot")
    EKOEquipmentSlotType SlotType = EKOEquipmentSlotType::Weapon;

    UPROPERTY(BlueprintReadOnly, Category = "KO|EquipmentSlot")
    FName EquippedItemId = NAME_None;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> EquipmentIconImage;

    UPROPERTY(BlueprintReadOnly, Category = "KO|EquipmentSlot")
    TObjectPtr<UKOInventoryComponent> InventoryComponent;

    UPROPERTY(EditDefaultsOnly, Category = "KO|Tooltip")
    TSubclassOf<UKOItemTooltipWidget> ItemTooltipWidgetClass;

private:
    void ResolveInventoryComponentIfNeeded();
    
    UKOEquipmentComponent* ResolveEquipmentComponent() const;

    bool CanAcceptItem(FName ItemId) const;
    
    // 무기 장착/해제
    bool ApplyEquipmentToComponent();

    void RefreshVisual();

    void RefreshTooltip();
};