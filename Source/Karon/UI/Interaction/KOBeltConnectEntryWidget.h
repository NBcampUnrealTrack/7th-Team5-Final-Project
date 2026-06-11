// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Subsystem/KOItemPortTypes.h"
#include "KOBeltConnectEntryWidget.generated.h"

class UButton;
class UTextBlock;
class UImage;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKOOnBeltConnectSlotClicked, FKOFactoryPortSlot, ClickedSlot);

/**
 * 벨트 연결 팝업의 슬롯 엔트리 한 칸(클릭 전용).
 * 슬롯 = (Kind, ItemId). 클릭 시 OnSlotClicked 로 해당 슬롯을 브로드캐스트한다.
 * 인벤토리 드래그앤드랍과 무관한 단순 버튼 엔트리(UKOFactorySlotWidget 재사용 안 함).
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOBeltConnectEntryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * 포트 슬롯 세팅(일반 포트 모델 — 빈 포트도 바인딩 가능).
     * @param InSlot      (Kind, PortIndex, ItemId 힌트). PortIndex 가 바인딩 키.
     * @param SlotNumber  그룹 내 1-based 표시 번호. SlotNumberText 에 표시.
     * @param InIcon      아이템 힌트 아이콘. nullptr(빈 포트)이면 EmptySlotIcon 사용.
     * @param bOccupied   true 면 이미 다른 벨트가 이 포트를 점유 → 버튼 비활성.
     */
    void SetupSlot(const FKOFactoryPortSlot& InSlot, int32 SlotNumber, const FText& InDisplayName, UTexture2D* InIcon, bool bOccupied = false);

    const FKOFactoryPortSlot& GetSlot() const { return PortSlot; }

    /** 엔트리 클릭 시 슬롯과 함께 브로드캐스트. */
    UPROPERTY(BlueprintAssignable, Category = "KO|UI|BeltConnect")
    FKOOnBeltConnectSlotClicked OnSlotClicked;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> SlotButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> NameText;

    /** 그룹 내 슬롯 번호(1,2,3,4 …) 표시용. 팩토리 슬롯형 레이아웃에서 사용. */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> SlotNumberText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> IconImage;

    /** 빈 슬롯에 표시할 아이콘(미지정 시 아이콘 숨김). 디자이너가 BP 에서 지정. */
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|BeltConnect")
    TObjectPtr<UTexture2D> EmptySlotIcon;

private:
    UFUNCTION()
    void HandleButtonClicked();

    FKOFactoryPortSlot PortSlot;
};
