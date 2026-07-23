// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/KOItemSlot.h"

#include "GMRouterSubsystem.h"
#include "Utility/Messaging/KOGMSInterface.h"
#include "KOInventoryWidget.generated.h"

class UKOInventoryComponent;
class UKOInventorySlotWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FKOOnInventorySlotClicked,
    int32, SlotIndex,
    const FKOItemSlot&, InSlot
);

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOInventoryWidget : public UUserWidget, public IKOGMSInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "KO|UI|Inventory")
    void SetInventoryComponent(UKOInventoryComponent* InComponent);

    UFUNCTION(BlueprintPure, Category = "KO|UI|Inventory")
    UKOInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }

    /** SlotWidget이 클릭됐을 때 부모(패널 등)로 버블업되는 이벤트. */
    UPROPERTY(BlueprintAssignable, Category = "KO|UI|Inventory")
    FKOOnInventorySlotClicked OnSlotClicked;

    /** SlotWidget이 우클릭됐을 때 부모(패널 등)로 버블업되는 이벤트. (장착 등 처리용) */
    UPROPERTY(BlueprintAssignable, Category = "KO|UI|Inventory")
    FKOOnInventorySlotClicked OnSlotRightClicked;

    /** 자식 SlotWidget이 호출. (C++ 내부에서 자동 라우팅됨) */
    void NotifySlotClicked(int32 SlotIndex, const FKOItemSlot& InSlot);

    /** 자식 SlotWidget이 우클릭 시 호출. (C++ 내부에서 자동 라우팅됨) */
    void NotifySlotRightClicked(int32 SlotIndex, const FKOItemSlot& InSlot);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    /** WBP에 WrapBox를 'SlotsPanel'이라는 이름으로 배치하면 자동 바인딩. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UWrapBox> SlotsPanel;

    /** 인스턴스화할 슬롯 위젯 클래스 (WBP_InventorySlot). */
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Inventory")
    TSubclassOf<UKOInventorySlotWidget> SlotWidgetClass;

    UPROPERTY(BlueprintReadOnly, Category = "KO|UI|Inventory")
    TObjectPtr<UKOInventoryComponent> InventoryComponent;

private:
    UFUNCTION()
    void OnInventoryChangedGMS(FGameplayTag Channel, const FInstancedStruct& Payload);

    void ResolveInventoryComponentIfNeeded();

    /** Slots 배열을 SlotWidgets에 반영. 필요한 만큼만 생성/제거. */
    void RebuildSlotWidgets();

    UPROPERTY(Transient)
    TArray<TObjectPtr<UKOInventorySlotWidget>> SlotWidgets;

    FGameplayMessageCallback InventoryChangedCallback;
    FGameplayMessageHandle   InventoryChangedHandle;
};
