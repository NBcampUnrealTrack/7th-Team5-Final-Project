// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Inventory/KOInventorySlotWidget.h"
#include "UI/Inventory/KOInventoryWidget.h"
#include "Items/KOItemLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "UI/Inventory/KOItemDragDropOperation.h"
#include "UI/Inventory/KOItemDragSource.h"
#include "UI/Inventory/KOInventoryWidget.h"
#include "Component/Inventory/KOInventoryComponent.h"

void UKOInventorySlotWidget::SetupSlot(UKOInventoryWidget* InOwningInventory, int32 InSlotIndex)
{
    OwningInventory = InOwningInventory;
    SlotIndex = InSlotIndex;
}

void UKOInventorySlotWidget::SetSlotData(const FKOItemSlot& InSlot)
{
    SlotData = InSlot;

    CachedDisplayName = FText::GetEmpty();
    CachedIcon = nullptr;

    if (SlotData.HasItem())
    {
        CachedDisplayName = UKOItemLibrary::GetDisplayName(this, SlotData.Kind, SlotData.ItemId);
        CachedIcon        = UKOItemLibrary::GetIcon(this, SlotData.Kind, SlotData.ItemId);
    }

    ApplyVisuals();
}

void UKOInventorySlotWidget::ApplyVisuals()
{
    const bool bHasItem = SlotData.HasItem();

    if (IconImage)
    {
        UTexture2D* TextureToShow = bHasItem ? CachedIcon.Get() : EmptySlotIcon.Get();

        if (TextureToShow)
        {
            IconImage->SetBrushFromTexture(TextureToShow);
            IconImage->SetDesiredSizeOverride(FVector2D(SlotIconSize, SlotIconSize));
            IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            IconImage->SetVisibility(ESlateVisibility::Hidden);
        }
    }

    if (CountText)
    {
        if (bHasItem && SlotData.Count > 1)
        {
            CountText->SetText(FText::AsNumber(SlotData.Count));
            CountText->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            CountText->SetText(FText::GetEmpty());
            CountText->SetVisibility(bHideCountWhenEmpty ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
        }
    }

    if (DisplayNameText)
    {
        DisplayNameText->SetText(CachedDisplayName);
        DisplayNameText->SetVisibility(bHasItem ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
    }
}

FReply UKOInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
    {
        if (UKOInventoryWidget* Owner = OwningInventory.Get())
        {
            Owner->NotifySlotClicked(SlotIndex, SlotData);
        }

        FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(
            InMouseEvent,
            this,
            EKeys::LeftMouseButton
        );

        return Reply.NativeReply;
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UKOInventorySlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
    UDragDropOperation*& OutOperation)
{
    Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
    
    if (!SlotData.HasItem())
    {
        return;
    }

    UKOInventoryComponent* InventoryComponent = nullptr;

    if (UKOInventoryWidget* Owner = OwningInventory.Get())
    {
        InventoryComponent = Owner->GetInventoryComponent();
    }

    UKOItemDragDropOperation* DragOp = UKOItemDragDropOperation::CreateItemDragOperation(
        this,
        SlotData,
        CachedDisplayName,
        CachedIcon.Get(),
        DragVisualSize,
        DragVisualOpacity,
        nullptr
    );
    if (DragOp && InventoryComponent)
    {
        UKOInventorySlotItemSource* Src = NewObject<UKOInventorySlotItemSource>(DragOp);
        Src->Inventory = InventoryComponent;
        Src->SlotIndex = SlotIndex;
        DragOp->Source = Src;
    }
    OutOperation = DragOp;
}

bool UKOInventorySlotWidget::NativeOnDrop(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    UKOItemDragDropOperation* DragOp = Cast<UKOItemDragDropOperation>(InOperation);
    if (!DragOp || !DragOp->HasItem())
    {
        return false;
    }

    UKOInventorySlotItemSource* InvSource = Cast<UKOInventorySlotItemSource>(DragOp->Source);
    if (!InvSource)
    {
        // 인벤토리 슬롯이 아닌 출발지(Factory 등)는 인벤토리 패널이 처리.
        return false;
    }

    UKOInventoryComponent* SourceInv = InvSource->GetInventory();
    if (!SourceInv)
    {
        return false;
    }

    UKOInventoryComponent* MyInv = nullptr;
    if (UKOInventoryWidget* Owner = OwningInventory.Get())
    {
        MyInv = Owner->GetInventoryComponent();
    }
    if (!MyInv || MyInv != SourceInv)
    {
        return false;
    }

    return MyInv->SwapSlots(InvSource->SlotIndex, SlotIndex);
}
