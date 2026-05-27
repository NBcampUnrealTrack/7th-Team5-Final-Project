// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Items/KOItemSlot.h"
#include "KOInventorySlotWidget.generated.h"

class UImage;
class UTextBlock;
class UTexture2D;
class UKOInventoryWidget;

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** 인벤토리 위젯이 슬롯 생성 직후 호출. 클릭 라우팅용 부모와 인덱스 주입. */
    void SetupSlot(UKOInventoryWidget* InOwningInventory, int32 InSlotIndex);

    /** 슬롯 데이터 갱신. 내부에서 IconImage/CountText/DisplayName 텍스트를 직접 갱신. */
    void SetSlotData(const FKOItemSlot& InSlot);

    const FKOItemSlot& GetSlotData() const { return SlotData; }
    int32 GetSlotIndex() const { return SlotIndex; }

protected:
    virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

    /** WBP에서 같은 이름으로 위젯을 만들면 자동 바인딩. 비주얼만 디자인. */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> IconImage;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> CountText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> DisplayNameText;

    /** 빈 슬롯일 때 IconImage에 그릴 텍스처. None이면 IconImage를 Hidden 처리. */
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Inventory")
    TObjectPtr<UTexture2D> EmptySlotIcon;

    /** 빈 슬롯일 때 CountText 숨김 여부 (true면 Collapsed). */
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Inventory")
    bool bHideCountWhenEmpty = true;

private:
    void ApplyVisuals();

    FKOItemSlot SlotData;
    int32 SlotIndex = INDEX_NONE;
    TWeakObjectPtr<UKOInventoryWidget> OwningInventory;

    FText CachedDisplayName;
    TObjectPtr<UTexture2D> CachedIcon;
};
