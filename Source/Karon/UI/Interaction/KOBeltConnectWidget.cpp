// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOBeltConnectWidget.h"

#include "UI/Interaction/KOBeltConnectEntryWidget.h"
#include "Subsystem/KOQuestGuideSubsystem.h"
#include "Building/KOBaseBuilding.h"
#include "Building/Conveyor/KOConveyorBelt.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
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

    AKOConveyorBelt* Belt = TargetBelt.Get();
    if (!Belt)
    {
        return;
    }

    // Output 선택 전에는 아무 output도 꺼내지 않도록 대기 상태로 만든다.
    Belt->BeginOutputPortSelection(Building);

    // 선택 레시피 기준 아이템 힌트 수집(미선택/없으면 빈 칸으로 표시될 뿐, 포트는 그대로 노출).
    TArray<FKOFactoryPortSlot> Slots;
    UKOItemLibrary::GatherFactoryPortSlots(this, Building, Slots);

    // 노출할 Kind 의 아이템 힌트만 PortIndex 순서대로 누적.
    TArray<FName> Hints;
    for (const FKOFactoryPortSlot& PortSlot : Slots)
    {
        if (PortSlot.Kind == EKOPortKind::Output)
        {
            Hints.Add(PortSlot.ItemId);
        }
    }
    
    if (EmptyText)
    {
        EmptyText->SetVisibility(Hints.Num() == 0 ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

        if (Hints.Num() == 0)
        {
            EmptyText->SetText(FText::FromString(TEXT("선택된 레시피가 없어 출력 아이템이 없습니다.")));
        }
    }

    BuildGroupEntries(EKOPortKind::Output, Hints, OutputSlotsPanel.Get(), Building);
}

void UKOBeltConnectWidget::BuildGroupEntries(EKOPortKind Kind, const TArray<FName>& ItemHints, UPanelWidget* Panel, AKOBaseBuilding* Building)
{
    if (!Panel || !EntryClass)
    {
        return;
    }

    // Output 아이템 힌트 수만큼 슬롯을 생성한다.
    // 슬롯은 물리 포트 점유가 아니라, 이 벨트가 꺼낼 아이템 필터다.
    const int32 Total = ItemHints.Num();
    for (int32 Index = 0; Index < Total; ++Index)
    {
        UKOBeltConnectEntryWidget* Entry = CreateWidget<UKOBeltConnectEntryWidget>(this, EntryClass);
        if (!Entry)
        {
            continue;
        }

        const FName ItemHint = ItemHints[Index];
        const FKOFactoryPortSlot PortSlot(Kind, Index, ItemHint);

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

        Entry->SetupSlot(PortSlot, Index + 1, DisplayName, Icon);
        Entry->OnSlotClicked.AddDynamic(this, &UKOBeltConnectWidget::HandleSlotClicked);

        Panel->AddChild(Entry);
        EntryWidgets.Add(Entry);
    }
}

void UKOBeltConnectWidget::HandleSlotClicked(FKOFactoryPortSlot ClickedSlot)
{
    // Output 슬롯을 선택하면 이 벨트가 꺼낼 아이템 필터로 저장한다.
    AKOConveyorBelt* Belt     = TargetBelt.Get();
    AKOBaseBuilding* Building  = TargetBuilding.Get();
    if (Belt && Building)
    {
        Belt->BindToMachinePort(Building, ClickedSlot);
        
        // 퀘스트
        if (ClickedSlot.Kind == EKOPortKind::Output)
        {
            if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
            {
                QuestGuide->NotifyConveyorOutputBound(ClickedSlot.ItemId);
            }
        }
    }

    DeactivateWidget();
}

void UKOBeltConnectWidget::NativeOnDeactivated()
{    
    for (UKOBeltConnectEntryWidget* Entry : EntryWidgets)
    {
        if (Entry)
        {
            Entry->OnSlotClicked.RemoveDynamic(this, &UKOBeltConnectWidget::HandleSlotClicked);
        }
    }
    EntryWidgets.Reset();
    if (OutputSlotsPanel) OutputSlotsPanel->ClearChildren();

    TargetBelt.Reset();
    TargetBuilding.Reset();

    Super::NativeOnDeactivated();
}
