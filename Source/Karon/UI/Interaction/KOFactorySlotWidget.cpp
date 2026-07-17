// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOFactorySlotWidget.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Component/Factory/KOEnergyProducerComponent.h"
#include "Component/Factory/KOFactoryProcessorComponent.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"
#include "Data/KODataTableTypes.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Subsystem/KOQuestGuideSubsystem.h"
#include "UI/Inventory/KOItemDragDropOperation.h"
#include "UI/Inventory/KOItemDragSource.h"
#include "UI/ItemTooltip/KOItemTooltipWidget.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

void UKOFactorySlotWidget::SetupFuelSlot(UKOEnergyProducerComponent* InProducer)
{
    Mode = EKOFactorySlotMode::Fuel;
    Producer = InProducer;
    Processor.Reset();
    SlotItemId = InProducer ? InProducer->GetAcceptedFuelItemId() : NAME_None;
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

bool UKOFactorySlotWidget::TryMoveInventorySlotToThis(UKOInventoryComponent* Inventory, int32 SlotIndex,
    const FKOItemSlot& InSlot)
{
    if (!Inventory)
    {
        return false;
    }

    if (!InSlot.HasItem() || InSlot.Kind != EKOSlotKind::Item)
    {
        return false;
    }

    const FName ItemId = InSlot.ItemId;
    const int32 Count = InSlot.Count;

    if (ItemId.IsNone() || Count <= 0)
    {
        return false;
    }

    if (Mode == EKOFactorySlotMode::Fuel)
    {
        UKOEnergyProducerComponent* Prod = Producer.Get();
        if (!Prod)
        {
            return false;
        }

        if (Prod->GetAcceptedFuelItemId().IsNone() || ItemId != Prod->GetAcceptedFuelItemId())
        {
            return false;
        }
    }
    else if (Mode == EKOFactorySlotMode::ProcessorInput)
    {
        if (!SlotItemId.IsNone() && ItemId != SlotItemId)
        {
            return false;
        }

        if (!Processor.IsValid())
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    const int32 Removed = Inventory->RemoveAtSlot(SlotIndex, Count);
    if (Removed <= 0)
    {
        return false;
    }

    int32 Rejected = Removed;

    if (Mode == EKOFactorySlotMode::Fuel)
    {
        UKOEnergyProducerComponent* Prod = Producer.Get();
        Rejected = Prod ? Prod->TryInsertFuel(ItemId, Removed) : Removed;
    }
    else if (Mode == EKOFactorySlotMode::ProcessorInput)
    {
        UKOFactoryProcessorComponent* Proc = Processor.Get();
        Rejected = Proc ? Proc->TryInsertItem(ItemId, Removed) : Removed;
    }

    if (Rejected > 0)
    {
        Inventory->TryAddItem(EKOSlotKind::Item, ItemId, Rejected);
    }

    RefreshFromComponent();

    return Removed - Rejected > 0;
}

void UKOFactorySlotWidget::ApplyVisual(FName ItemId, int32 Count)
{
    const bool bHasItem = !ItemId.IsNone() && Count > 0;

    bool bShowPreview = false;
    if (!bHasItem && !SlotItemId.IsNone())
    {
        if (Mode == EKOFactorySlotMode::Fuel)
        {
            bShowPreview = true;
        }
        else if (Mode == EKOFactorySlotMode::ProcessorInput || Mode == EKOFactorySlotMode::ProcessorOutput)
        {
            if (UKOFactoryProcessorComponent* Proc = Processor.Get())
            {
                const FName SelectedId = Proc->GetSelectedRecipe();
                if (!SelectedId.IsNone())
                {
                    if (const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this))
                    {
                        if (const FKORecipeRow* Recipe = LoadSub->FindRecipeRow(SelectedId))
                        {
                            const TMap<FGameplayTag, int32>& Map =
                                (Mode == EKOFactorySlotMode::ProcessorInput) ? Recipe->Inputs : Recipe->Outputs;
                            for (const TPair<FGameplayTag, int32>& Pair : Map)
                            {
                                if (LoadSub->FindItemIdByTag(Pair.Key) == SlotItemId)
                                {
                                    bShowPreview = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (IconImage)
    {
        UTexture2D* Icon = nullptr;
        if (bHasItem || bShowPreview)
        {
            Icon = UKOItemLibrary::GetIcon(this, EKOSlotKind::Item, ItemId.IsNone() ? SlotItemId : ItemId);
        }
        else
        {
            Icon = EmptySlotIcon.Get();
        }

        if (Icon)
        {
            IconImage->SetBrushFromTexture(Icon);
            IconImage->SetDesiredSizeOverride(FVector2D(SlotIconSize, SlotIconSize));
            IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
            IconImage->SetRenderOpacity(bShowPreview ? PreviewOpacity : 1.f);
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
            CountText->SetRenderOpacity(1.f);
        }
        else
        {
            CountText->SetText(FText::GetEmpty());
            CountText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
        
    // Tooltip 갱신
    const FName TooltipItemId = ItemId.IsNone() ? SlotItemId : ItemId;

    if (TooltipItemId.IsNone() || !TooltipClass)
    {
        SetToolTip(nullptr);
        return;
    }

    UKOItemTooltipWidget* Tooltip =
        CreateWidget<UKOItemTooltipWidget>(GetOwningPlayer(), TooltipClass);

    if (Tooltip)
    {
        Tooltip->SetSlot(EKOSlotKind::Item, TooltipItemId);
        SetToolTip(Tooltip);
    }
}

bool UKOFactorySlotWidget::MoveCurrentSlotItemToInventory()
{
    UKOInventoryComponent* Inventory = ResolvePlayerInventory();
    if (!Inventory)
    {
        return false;
    }

    FName ItemId = NAME_None;
    const int32 Extracted = ExtractCurrentSlotItem(ItemId);

    if (ItemId.IsNone() || Extracted <= 0)
    {
        return false;
    }

    const int32 Rejected = Inventory->TryAddItem(EKOSlotKind::Item, ItemId, Extracted);

    if (Rejected > 0)
    {
        RestoreCurrentSlotItem(ItemId, Rejected);
    }

    const int32 Accepted = Extracted - Rejected;

    RefreshFromComponent();

    return Accepted > 0;
}

int32 UKOFactorySlotWidget::ExtractCurrentSlotItem(FName& OutItemId)
{
    OutItemId = NAME_None;

    if (CachedCount <= 0)
    {
        return 0;
    }

    switch (Mode)
    {
    case EKOFactorySlotMode::Fuel:
        {
            UKOEnergyProducerComponent* Prod = Producer.Get();
            if (!Prod)
            {
                return 0;
            }

            OutItemId = Prod->GetFuelItemId();

            if (OutItemId.IsNone())
            {
                return 0;
            }

            return Prod->TryExtractFuel(CachedCount);
        }

    case EKOFactorySlotMode::ProcessorInput:
        {
            UKOFactoryProcessorComponent* Proc = Processor.Get();
            if (!Proc || SlotItemId.IsNone())
            {
                return 0;
            }

            OutItemId = SlotItemId;
            return Proc->TryExtractInputItem(SlotItemId, CachedCount);
        }

    case EKOFactorySlotMode::ProcessorOutput:
        {
            UKOFactoryProcessorComponent* Proc = Processor.Get();
            if (!Proc || SlotItemId.IsNone())
            {
                return 0;
            }

            OutItemId = SlotItemId;
            return Proc->TryExtractItem(SlotItemId, CachedCount);
        }

    default:
        return 0;
    }
}

void UKOFactorySlotWidget::RestoreCurrentSlotItem(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return;
    }

    switch (Mode)
    {
    case EKOFactorySlotMode::Fuel:
        {
            if (UKOEnergyProducerComponent* Prod = Producer.Get())
            {
                Prod->RestoreFuelBuffer(ItemId, Count);
            }
            break;
        }

    case EKOFactorySlotMode::ProcessorInput:
        {
            if (UKOFactoryProcessorComponent* Proc = Processor.Get())
            {
                Proc->RestoreInputBuffer(ItemId, Count);
            }
            break;
        }

    case EKOFactorySlotMode::ProcessorOutput:
        {
            if (UKOFactoryProcessorComponent* Proc = Processor.Get())
            {
                Proc->RestoreOutputBuffer(ItemId, Count);
            }
            break;
        }

    default:
        break;
    }
}

UKOInventoryComponent* UKOFactorySlotWidget::ResolvePlayerInventory() const
{
    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return nullptr;
    }

    if (UKOInventoryComponent* Inventory = PC->FindComponentByClass<UKOInventoryComponent>())
    {
        return Inventory;
    }

    if (APawn* Pawn = PC->GetPawn())
    {
        return Pawn->FindComponentByClass<UKOInventoryComponent>();
    }

    return nullptr;
}

FReply UKOFactorySlotWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
    {
        if (MoveCurrentSlotItemToInventory())
        {
            return FReply::Handled();
        }
    }
    
    const bool bDragSourceMode =
        Mode == EKOFactorySlotMode::ProcessorOutput ||
        Mode == EKOFactorySlotMode::ProcessorInput  ||
        Mode == EKOFactorySlotMode::Fuel;

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
    const bool bIsFuel   = Mode == EKOFactorySlotMode::Fuel;
    if (!bIsInput && !bIsOutput && !bIsFuel)
    {
        return;
    }

    FName PayloadItemId = NAME_None;
    int32 PayloadCount  = 0;

    if (bIsFuel)
    {
        UKOEnergyProducerComponent* Prod = Producer.Get();
        if (!Prod || Prod->GetFuelItemId().IsNone() || Prod->GetFuelCount() <= 0)
        {
            return;
        }
        PayloadItemId = Prod->GetFuelItemId();
        PayloadCount  = Prod->GetFuelCount();
    }
    else
    {
        UKOFactoryProcessorComponent* Proc = Processor.Get();
        if (!Proc || SlotItemId.IsNone() || CachedCount <= 0)
        {
            return;
        }
        PayloadItemId = SlotItemId;
        PayloadCount  = CachedCount;
    }

    FKOItemSlot Payload;
    Payload.Kind   = EKOSlotKind::Item;
    Payload.ItemId = PayloadItemId;
    Payload.Count  = PayloadCount;

    const FText DisplayName = UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Item, PayloadItemId);
    UTexture2D* Icon        = UKOItemLibrary::GetIcon(this, EKOSlotKind::Item, PayloadItemId);

    UKOItemDragDropOperation* DragOp = UKOItemDragDropOperation::CreateItemDragOperation(
        this,
        Payload,
        DisplayName,
        Icon,
        DragVisualSize,
        DragVisualOpacity,
        nullptr
    );
    if (!DragOp)
    {
        return;
    }

    if (bIsFuel)
    {
        UKOProducerFuelItemSource* Src = NewObject<UKOProducerFuelItemSource>(DragOp);
        Src->Producer = Producer;
        DragOp->Source = Src;
    }
    else if (bIsInput)
    {
        UKOProcessorInputItemSource* Src = NewObject<UKOProcessorInputItemSource>(DragOp);
        Src->Processor = Processor;
        DragOp->Source = Src;
    }
    else // bIsOutput
    {
        UKOProcessorOutputItemSource* Src = NewObject<UKOProcessorOutputItemSource>(DragOp);
        Src->Processor = Processor;
        DragOp->Source = Src;
    }

    OutOperation = DragOp;
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

    // 인벤토리 슬롯에서 출발한 드래그만 받음.
    UKOItemDragSource* Source = DragOp->Source;
    if (!Source || !Source->IsInventorySource())
    {
        return false;
    }

    const FName ItemId = DragOp->GetItemId();
    const int32 Count  = DragOp->GetCount();
    if (ItemId.IsNone() || Count <= 0)
    {
        return false;
    }

    if (Mode == EKOFactorySlotMode::ProcessorInput && !SlotItemId.IsNone() && ItemId != SlotItemId)
    {
        return false;
    }

    int32 Remaining = Count;
    if (Mode == EKOFactorySlotMode::Fuel)
    {
        UKOEnergyProducerComponent* Prod = Producer.Get();
        if (!Prod) return false;
        UE_LOG(LogTemp, Log, TEXT("[FuelSlot] NativeOnDrop: ItemId='%s', Count=%d, AcceptedFuel='%s'"),
            *ItemId.ToString(), Count, *Prod->GetFuelItemId().ToString());
        Remaining = Prod->TryInsertFuel(ItemId, Count);
    }
    else // ProcessorInput
    {
        UKOFactoryProcessorComponent* Proc = Processor.Get();
        if (!Proc) return false;
        Remaining = Proc->TryInsertItem(ItemId, Count);
    }

    const int32 Inserted = Count - Remaining;
    if (Inserted <= 0)
    {
        return false;
    }

    Source->Extract(ItemId, Inserted);
    RefreshFromComponent();
    return true;
}
