#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/KODataTableTypes.h"
#include "KOEquipmentSlotWidget.generated.h"

class UImage;
class UWidgetSwitcher;
class UDragDropOperation;
class UKOInventoryComponent;
class UKOItemTooltipWidget;
class UKOEquipmentComponent;
class UKOItemDragSource;

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

    /** 인벤토리 슬롯의 우클릭 등, 드래그가 아닌 경로에서 장착을 시도할 때 사용. */
    UFUNCTION(BlueprintCallable, Category = "KO|EquipmentSlot")
    bool TryEquipItemFromInventorySlot(int32 InventorySlotIndex, FName ItemId);

    /** 드래그 출발지(UKOEquipmentSlotItemSource)에서 호출: 장착 해제하고 실제로 빠져나간 수량 반환. */
    int32 ExtractEquippedItem(FName ItemId, int32 Count);

    /** 드래그 도착지가 거부했을 때 원래대로 재장착. */
    void RestoreEquippedItem(FName ItemId, int32 Count);

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

    virtual void NativeOnDragDetected(
        const FGeometry& InGeometry,
        const FPointerEvent& InMouseEvent,
        UDragDropOperation*& OutOperation
    ) override;

protected:    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KO|EquipmentSlot")
    EKOEquipmentSlotType SlotType = EKOEquipmentSlotType::Weapon;

    UPROPERTY(BlueprintReadOnly, Category = "KO|EquipmentSlot")
    FName EquippedItemId = NAME_None;
    
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UWidgetSwitcher> EquipmentStateSwitcher;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> EquipmentIconImage;

    UPROPERTY(BlueprintReadOnly, Category = "KO|EquipmentSlot")
    TObjectPtr<UKOInventoryComponent> InventoryComponent;

    UPROPERTY(EditDefaultsOnly, Category = "KO|Tooltip")
    TSubclassOf<UKOItemTooltipWidget> ItemTooltipWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KO|EquipmentSlot|Drag")
    FVector2D DragVisualSize = FVector2D(64.0f, 64.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "KO|EquipmentSlot|Drag")
    float DragVisualOpacity = 0.85f;

private:
    void ResolveInventoryComponentIfNeeded();

    UKOEquipmentComponent* ResolveEquipmentComponent() const;

    bool CanAcceptItem(FName ItemId) const;

    /** 스킬 GA 활성 중이면 EquipmentComponent의 OnEquipmentChangeBlocked를 브로드캐스트하고 true 반환. */
    bool NotifyIfBlockedBySkill(UKOEquipmentComponent* EquipmentComponent) const;

    // 무기 장착/해제
    bool ApplyEquipmentToComponent();

    /** NativeOnDrop과 우클릭 장착 경로가 공유하는 실제 장착 처리. Source->Extract로 아이템을 가져와 장착, 실패 시 롤백. */
    bool TryEquipFromSource(FName ItemId, UKOItemDragSource* Source);

    void RefreshVisual();

    void RefreshTooltip();
};