// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "Subsystem/KOItemPortTypes.h"
#include "KOBeltConnectWidget.generated.h"

class AKOBaseBuilding;
class AKOConveyorBelt;
class UPanelWidget;
class UTextBlock;
class UKOBeltConnectEntryWidget;

/**
 * 벨트를 공장 포트(아이템 종류 단위 슬롯)에 연결하는 팝업.
 * - 오픈 직후 SetupConnection(belt, building) 으로 컨텍스트 주입 → 슬롯 리스트 빌드.
 * - 슬롯 = 공장 매칭 레시피의 입력/출력 ItemId (UKOItemLibrary::GatherFactoryPortSlots).
 * - 슬롯 선택 시(Phase1) 로그+닫기. Phase2 에서 벨트-포트 바인딩으로 교체 예정.
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOBeltConnectWidget : public UKOActivatableWidget
{
    GENERATED_BODY()

public:
    UKOBeltConnectWidget();

    /** 오픈 직후 호출(컨텍스트 주입). 대상 벨트/공장 저장 후 슬롯 리스트 빌드. */
    void SetupConnection(AKOConveyorBelt* InBelt, AKOBaseBuilding* InBuilding);

protected:
    virtual void NativeOnDeactivated() override;

    void BuildSlotEntries();

    /**
     * 한 그룹(Input/Output) 패널을 현재 레시피의 아이템 수만큼 채운다.
     * ItemHints[i]는 포트 i의 표시용 아이템이며, 빈 슬롯은 생성하지 않는다.
     */
    void BuildGroupEntries(EKOPortKind Kind, const TArray<FName>& ItemHints, UPanelWidget* Panel, AKOBaseBuilding* Building);

    UFUNCTION()
    void HandleSlotClicked(FKOFactoryPortSlot ClickedSlot);

    /** 출력 슬롯 엔트리를 담을 패널. */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UPanelWidget> OutputSlotsPanel;

    /** 제목/안내 텍스트(선택). */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TitleText;

    /** 슬롯 엔트리 위젯 클래스(BP 에서 지정). */
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|BeltConnect")
    TSubclassOf<UKOBeltConnectEntryWidget> EntryClass;

private:
    TWeakObjectPtr<AKOConveyorBelt> TargetBelt;
    TWeakObjectPtr<AKOBaseBuilding> TargetBuilding;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UKOBeltConnectEntryWidget>> EntryWidgets;
};
