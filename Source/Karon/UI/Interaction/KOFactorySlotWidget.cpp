// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOFactorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Component/KOEnergyProducerComponent.h"
#include "Component/KOFactoryProcessorComponent.h"
#include "Component/KOInventoryComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
#include "UI/KOItemDragDropOperation.h"

void UKOFactorySlotWidget::SetupFuelSlot(UKOEnergyProducerComponent* InProducer)
{
    Mode = EKOFactorySlotMode::Fuel;
    Producer = InProducer;
    Processor.Reset();
    SlotItemId = NAME_None;
    RefreshFromComponent();
}

void UKOFactorySlotWidget::SetupInputSlot(UKOFactoryProcessorComponent* InProcessor, FName InItemId)
{
    Mode = EKOFactorySlotMode::ProcessorInput;
    Processor = InProcessor;
    Producer.Reset();
    SlotItemId = InItemId;
    RefreshFromComponent();
}

void UKOFactorySlotWidget::SetupOutputSlot(UKOFactoryProcessorComponent* InProcessor, FName InItemId)
{
    Mode = EKOFactorySlotMode::ProcessorOutput;
    Processor = InProcessor;
    Producer.Reset();
    SlotItemId = InItemId;
    RefreshFromComponent();
}

void UKOFactorySlotWidget::RefreshFromComponent()
{
    FName DisplayItemId = NAME_None;
    int32 DisplayCount  = 0;

    switch (Mode)
    {
    case EKOFactorySlotMode::Fuel:
        if (UKOEnergyProducerComponent* Prod = Producer.Get())
        {
            DisplayItemId = Prod->GetFuelItemId();
            DisplayCount  = Prod->GetFuelCount();
        }
        break;

    case EKOFactorySlotMode::ProcessorInput:
        if (UKOFactoryProcessorComponent* Proc = Processor.Get())
        {
            DisplayItemId = SlotItemId;
            if (const int32* Found = Proc->GetInputBuffer().Find(SlotItemId))
            {
                DisplayCount = *Found;
            }
        }
        break;

    case EKOFactorySlotMode::ProcessorOutput:
        if (UKOFactoryProcessorComponent* Proc = Processor.Get())
        {
            DisplayItemId = SlotItemId;
            if (const int32* Found = Proc->GetOutputBuffer().Find(SlotItemId))
            {
                DisplayCount = *Found;
            }
        }
        break;
    }

    CachedCount = DisplayCount;
    ApplyVisual(DisplayItemId, DisplayCount);
}

void UKOFactorySlotWidget::ApplyVisual(FName ItemId, int32 Count)
{
    const bool bHasItem = !ItemId.IsNone() && Count > 0;

    if (IconImage)
    {
        UTexture2D* Icon = bHasItem
            ? UKOItemLibrary::GetIcon(this, EKOSlotKind::Item, ItemId)
            : EmptySlotIcon.Get();

        if (Icon)
        {
            IconImage->SetBrushFromTexture(Icon);
            IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            IconImage->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (CountText)
    {
        if (bHasItem)
        {
            CountText->SetText(FText::AsNumber(Count));
            CountText->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            CountText->SetText(FText::GetEmpty());
            CountText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}

FReply UKOFactorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    const bool bDragSourceMode =
        Mode == EKOFactorySlotMode::ProcessorOutput ||
        Mode == EKOFactorySlotMode::ProcessorInput;

    if (bDragSourceMode && InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(
            InMouseEvent,
            this,
            EKeys::LeftMouseButton
        );
        return Reply.NativeReply;
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UKOFactorySlotWidget::NativeOnDragDetected(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent,
    UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

    const bool bIsInput  = Mode == EKOFactorySlotMode::ProcessorInput;
    const bool bIsOutput = Mode == EKOFactorySlotMode::ProcessorOutput;
    if (!bIsInput && !bIsOutput)
    {
        return;
    }

    UKOFactoryProcessorComponent* Proc = Processor.Get();
    if (!Proc || SlotItemId.IsNone() || CachedCount <= 0)
    {
        return;
    }

    FKOItemSlot Payload;
    Payload.Kind   = EKOSlotKind::Item;
    Payload.ItemId = SlotItemId;
    Payload.Count  = CachedCount;

    const FText DisplayName = UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Item, SlotItemId);
    UTexture2D* Icon        = UKOItemLibrary::GetIcon(this, EKOSlotKind::Item, SlotItemId);

    OutOperation = UKOItemDragDropOperation::CreateItemDragOperation(
        this,
        Payload,
        DisplayName,
        Icon,
        DragVisualSize,
        DragVisualOpacity,
        INDEX_NONE,
        nullptr,
        Proc,
        bIsInput
    );
}

bool UKOFactorySlotWidget::NativeOnDrop(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    UKOItemDragDropOperation* DragOp = Cast<UKOItemDragDropOperation>(InOperation);
    if (!DragOp || !DragOp->HasItem())
    {
        return false;
    }

    // 출력 슬롯은 드롭 대상이 아니다.
    if (Mode == EKOFactorySlotMode::ProcessorOutput)
    {
        return false;
    }

    // 일반 아이템(EKOSlotKind::Item) 만 받음. Factory 카드는 거부.
    if (DragOp->ItemSlot.Kind != EKOSlotKind::Item)
    {
        return false;
    }

    // Processor에서 출발한 드래그는 인벤토리만 받음. 슬롯 간 직접 이동 금지.
    if (DragOp->SourceProcessor != nullptr)
    {
        return false;
    }

    UKOInventoryComponent* SourceInv = DragOp->SourceInventoryComponent;
    const int32 SourceSlotIdx        = DragOp->SourceSlotIndex;
    const FName ItemId               = DragOp->GetItemId();
    const int32 Count                = DragOp->GetCount();
    if (!SourceInv || SourceSlotIdx == INDEX_NONE || ItemId.IsNone() || Count <= 0)
    {
        return false;
    }

    int32 Remaining = Count;

    if (Mode == EKOFactorySlotMode::Fuel)
    {
        UKOEnergyProducerComponent* Prod = Producer.Get();
        if (!Prod)
        {
            return false;
        }
        Remaining = Prod->TryInsertFuel(ItemId, Count);
    }
    else // ProcessorInput
    {
        UKOFactoryProcessorComponent* Proc = Processor.Get();
        if (!Proc)
        {
            return false;
        }
        // 이 슬롯이 특정 ItemId에 바인딩돼 있으면 다른 아이템 거부
        if (!SlotItemId.IsNone() && ItemId != SlotItemId)
        {
            return false;
        }
        Remaining = Proc->TryInsertItem(ItemId, Count);
    }

    const int32 Inserted = Count - Remaining;
    if (Inserted <= 0)
    {
        return false;
    }

    SourceInv->RemoveAtSlot(SourceSlotIdx, Inserted);
    RefreshFromComponent();
    return true;
}
