// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/KOItemSlot.h"
#include "Messaging/KOGMSInterface.h"
#include "GMRouterSubsystem.h"
#include "KOInventoryWidget.generated.h"

class UKOInventoryComponent;
class UKOInventorySlotWidget;
class UPanelWidget;

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

    /** 자식 SlotWidget이 호출. (C++ 내부에서 자동 라우팅됨) */
    void NotifySlotClicked(int32 SlotIndex, const FKOItemSlot& InSlot);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    /** WBP에 같은 이름의 패널(UniformGridPanel / WrapBox / GridPanel 등)을 배치하면 자동 바인딩. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPanelWidget> SlotsPanel;

    /** 인스턴스화할 슬롯 위젯 클래스 (WBP_InventorySlot). */
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Inventory")
    TSubclassOf<UKOInventorySlotWidget> SlotWidgetClass;

    /** UniformGridPanel일 때 사용할 한 행당 칸 수. 다른 패널 타입에서는 무시. */
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Inventory", meta = (ClampMin = "1"))
    int32 ColumnsPerRow = 5;

    UPROPERTY(BlueprintReadOnly, Category = "KO|UI|Inventory")
    TObjectPtr<UKOInventoryComponent> InventoryComponent;

private:
    UFUNCTION()
    void OnInventoryChangedGMS(FGameplayTag Channel, const FInstancedStruct& Payload);

    void ResolveInventoryComponentIfNeeded();

    /** Slots 배열을 SlotWidgets에 반영. 필요한 만큼만 생성/제거. */
    void RebuildSlotWidgets();

    /** Index 위치의 SlotWidget을 패널에 부착. UniformGridPanel이면 Row/Col 자동 설정. */
    void AttachSlotWidgetToPanel(UKOInventorySlotWidget* SlotWidget, int32 Index);

    UPROPERTY(Transient)
    TArray<TObjectPtr<UKOInventorySlotWidget>> SlotWidgets;

    FGameplayMessageCallback InventoryChangedCallback;
    FGameplayMessageHandle   InventoryChangedHandle;
};
