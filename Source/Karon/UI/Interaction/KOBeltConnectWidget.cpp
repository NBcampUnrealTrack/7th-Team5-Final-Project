// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOBeltConnectWidget.h"

#include "UI/Interaction/KOBeltConnectEntryWidget.h"
#include "Building/KOBaseBuilding.h"
#include "Building/Conveyor/KOConveyorBelt.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
#include "Subsystem/KOConveyorSubsystem.h"
#include "Data/KODataTableTypes.h"

UKOBeltConnectWidget::UKOBeltConnectWidget()
{
    InputMode = EKOUIInputMode::Menu;
    bIsBackHandler = true; // Back(ESC) 시 자동 Deactivate.
}

void UKOBeltConnectWidget::SetupConnection(AKOConveyorBelt* InBelt, AKOBaseBuilding* InBuilding)
{
    TargetBelt     = InBelt;
    TargetBuilding = InBuilding;
    BuildSlotEntries();
}

void UKOBeltConnectWidget::BuildSlotEntries()
{
    if (InputSlotsPanel)  InputSlotsPanel->ClearChildren();
    if (OutputSlotsPanel) OutputSlotsPanel->ClearChildren();
    EntryWidgets.Reset();

    AKOBaseBuilding* Building = TargetBuilding.Get();
    if (!Building || !EntryClass)
    {
        return;
    }

    // 타이틀 = 대상 공장 이름(없으면 FactoryId). 기본 placeholder 텍스트 대체.
    if (TitleText)
    {
        const FKOFactoryRow* Row = Building->GetFactoryRow();
        TitleText->SetText((Row && !Row->DisplayName.IsEmpty())
            ? Row->DisplayName
            : FText::FromName(Building->GetFactoryId()));
    }

    // 벨트 설치 방향으로 연결 가능한 Kind 결정 — 그 그룹만 노출(방향과 모순된 선택 차단).
    // 출구가 머신 향함 → 머신 Input 에 공급, 입구가 머신 향함 → 머신 Output 에서 받음.
    AKOConveyorBelt* Belt = TargetBelt.Get();
    EKOPortKind ValidKind = EKOPortKind::Input;
    if (!Belt || !Belt->GetConnectablePortKind(Building, ValidKind))
    {
        return; // 흐름상 머신에 안 닿음 — 정상 트리거 경로에선 발생하지 않음.
    }
    
    Belt->BeginMachinePortSelection(Building, ValidKind);

    // 선택 레시피 기준 아이템 힌트 수집(미선택/없으면 빈 칸으로 표시될 뿐, 포트는 그대로 노출).
    TArray<FKOFactoryPortSlot> Slots;
    UKOItemLibrary::GatherFactoryPortSlots(this, Building, Slots);

    // 노출할 Kind 의 아이템 힌트만 PortIndex 순서대로 누적.
    TArray<FName> Hints;
    for (const FKOFactoryPortSlot& PortSlot : Slots)
    {
        if (PortSlot.Kind == ValidKind)
        {
            Hints.Add(PortSlot.ItemId);
        }
    }

    UPanelWidget* Panel = (ValidKind == EKOPortKind::Input) ? InputSlotsPanel.Get() : OutputSlotsPanel.Get();
    BuildGroupEntries(ValidKind, Hints, Panel, Building);
}

void UKOBeltConnectWidget::BuildGroupEntries(EKOPortKind Kind, const TArray<FName>& ItemHints, UPanelWidget* Panel, AKOBaseBuilding* Building)
{
    if (!Panel || !EntryClass)
    {
        return;
    }

    UKOConveyorSubsystem* ConveyorSub = UKOConveyorSubsystem::Get(this);

    // 일반 포트: 고정 칸 수와 힌트 수 중 큰 값만큼 포트 생성. 모든 포트가 바인딩 가능(빈 포트 포함).
    const int32 Total = ItemHints.Num();
    for (int32 Index = 0; Index < Total; ++Index)
    {
        UKOBeltConnectEntryWidget* Entry = CreateWidget<UKOBeltConnectEntryWidget>(this, EntryClass);
        if (!Entry)
        {
            continue;
        }

        const FName ItemHint = ItemHints.IsValidIndex(Index) ? ItemHints[Index] : NAME_None;
        const FKOFactoryPortSlot PortSlot(Kind, Index, ItemHint);

        // 이미 다른 벨트가 이 포트(Kind, Index)를 점유했으면 비활성으로 표시.
        const bool bOccupied = ConveyorSub && ConveyorSub->IsSlotBound(Building, Kind, Index);

        FText DisplayName;
        UTexture2D* Icon = nullptr;
        if (!ItemHint.IsNone())
        {
            DisplayName = UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Item, ItemHint);
            if (DisplayName.IsEmpty())
            {
                DisplayName = FText::FromName(ItemHint);
            }
            Icon = UKOItemLibrary::GetIcon(this, EKOSlotKind::Item, ItemHint);
        }

        Entry->SetupSlot(PortSlot, Index + 1, DisplayName, Icon, bOccupied);
        Entry->OnSlotClicked.AddDynamic(this, &UKOBeltConnectWidget::HandleSlotClicked);

        Panel->AddChild(Entry);
        EntryWidgets.Add(Entry);
    }
}

void UKOBeltConnectWidget::HandleSlotClicked(FKOFactoryPortSlot ClickedSlot)
{
    // 선택한 슬롯에 벨트를 실제 바인딩한 뒤 닫는다(점유 표시는 다음 오픈 시 반영).
    AKOConveyorBelt* Belt     = TargetBelt.Get();
    AKOBaseBuilding* Building  = TargetBuilding.Get();
    if (Belt && Building)
    {
        Belt->BindToMachinePort(Building, ClickedSlot);
    }

    DeactivateWidget(); // 자기 닫기(스택에서 제거).
}

void UKOBeltConnectWidget::NativeOnDeactivated()
{
    if (AKOConveyorBelt* Belt = TargetBelt.Get())
    {
        if (!Belt->HasSelectedPort())
        {
            Belt->ClearMachinePortBinding();
        }
    }
    
    for (UKOBeltConnectEntryWidget* Entry : EntryWidgets)
    {
        if (Entry)
        {
            Entry->OnSlotClicked.RemoveDynamic(this, &UKOBeltConnectWidget::HandleSlotClicked);
        }
    }
    EntryWidgets.Reset();
    if (InputSlotsPanel)  InputSlotsPanel->ClearChildren();
    if (OutputSlotsPanel) OutputSlotsPanel->ClearChildren();

    TargetBelt.Reset();
    TargetBuilding.Reset();

    Super::NativeOnDeactivated();
}
