// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "Items/KOItemSlot.h"
#include "KOInventoryPanelWidget.generated.h"

class UKOInventoryWidget;
class UKOInventoryComponent;

/**
 * 인벤토리 화면 컨테이너. CommonActivatableWidget 스택에 push/pop되는 단위.
 * 안쪽에 재사용 가능한 KOInventoryWidget을 BindWidget으로 보유하고,
 * 슬롯 클릭/입력 등의 UI 로직을 C++에서 처리한다.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOInventoryPanelWidget : public UKOActivatableWidget
{ 
    GENERATED_BODY()

public:
    UKOInventoryPanelWidget();
    
    /** 외부에서 InventoryComponent를 명시적으로 주입하고 싶을 때 사용. (없으면 PC/Pawn 자동 폴백) */
    UFUNCTION(BlueprintCallable, Category = "KO|UI|Inventory")
    void SetInventoryComponent(UKOInventoryComponent* InComponent);

    UKOInventoryWidget* GetInventoryWidget() const { return InventoryWidget; }

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** WBP에 KOInventoryWidget 인스턴스를 'InventoryWidget'이라는 이름으로 배치. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UKOInventoryWidget> InventoryWidget;

    /** 슬롯 클릭 시 호출. 자식 클래스/BP에서 오버라이드해 사용/장착/툴팁 등 분기. */
    UFUNCTION(BlueprintNativeEvent, Category = "KO|UI|Inventory")
    void OnSlotClicked(int32 SlotIndex, const FKOItemSlot& InSlot);
    virtual void OnSlotClicked_Implementation(int32 SlotIndex, const FKOItemSlot& InSlot);

private:
    UFUNCTION()
    void HandleSlotClicked(int32 SlotIndex, const FKOItemSlot& InSlot);
};
