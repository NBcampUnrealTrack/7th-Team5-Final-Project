// Copyright Karon Team 5. All Rights Reserved.
#include "UI/KOInventoryWidget.h"
#include "UI/KOInventorySlotWidget.h"
#include "UI/KOItemDragDropOperation.h"
#include "UI/KOItemDragSource.h"
#include "Component/KOInventoryComponent.h"
#include "Messaging/KOMessageTypes.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "StructUtils/InstancedStruct.h"
#include "GameFramework/PlayerController.h"
#include "Components/WrapBox.h"

void UKOInventoryWidget::SetInventoryComponent(UKOInventoryComponent* InComponent)
{
    InventoryComponent = InComponent;
    RebuildSlotWidgets();
}

void UKOInventoryWidget::NotifySlotClicked(int32 SlotIndex, const FKOItemSlot& InSlot)
{
    OnSlotClicked.Broadcast(SlotIndex, InSlot);
}

void UKOInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    ResolveInventoryComponentIfNeeded();

    InventoryChangedCallback.BindDynamic(this, &UKOInventoryWidget::OnInventoryChangedGMS);
    InventoryChangedHandle = Subscribe(KOGameplayTags::Data_Message_Inventory_Changed, InventoryChangedCallback);

    RebuildSlotWidgets();
}

void UKOInventoryWidget::NativeDestruct()
{
    Unsubscribe(InventoryChangedHandle);
    InventoryChangedHandle = FGameplayMessageHandle();
    InventoryChangedCallback.Clear();

    Super::NativeDestruct();
}

bool UKOInventoryWidget::NativeOnDrop(
    const FGeometry& InGeometry,
    const FDragDropEvent& InDragDropEvent,
    UDragDropOperation* InOperation)
{
    UKOItemDragDropOperation* DragOp = Cast<UKOItemDragDropOperation>(InOperation);
    if (!DragOp || !DragOp->HasItem())
    {
        return false;
    }

    UKOItemDragSource* Source = DragOp->Source;
    if (!Source || Source->IsInventorySource())
    {
        // 인벤토리에서 인벤토리로의 드래그는 슬롯 위젯이 swap으로 처리.
        return false;
    }

    if (!InventoryComponent)
    {
        return false;
    }

    const FName ItemId = DragOp->GetItemId();
    const int32 Count  = DragOp->GetCount();
    if (ItemId.IsNone() || Count <= 0)
    {
        return false;
    }

    const int32 Extracted = Source->Extract(ItemId, Count);
    if (Extracted <= 0)
    {
        return false;
    }

    const int32 Rejected = InventoryComponent->TryAddItem(EKOSlotKind::Item, ItemId, Extracted);
    if (Rejected > 0)
    {
        Source->Restore(ItemId, Rejected);
    }
    return true;
}

void UKOInventoryWidget::OnInventoryChangedGMS(FGameplayTag Channel, const FInstancedStruct& Payload)
{
    RebuildSlotWidgets();
}

void UKOInventoryWidget::ResolveInventoryComponentIfNeeded()
{
    if (InventoryComponent)
    {
        return;
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC)
    {
        return;
    }

    InventoryComponent = PC->FindComponentByClass<UKOInventoryComponent>();
    if (!InventoryComponent)
    {
        if (APawn* Pawn = PC->GetPawn())
        {
            InventoryComponent = Pawn->FindComponentByClass<UKOInventoryComponent>();
        }
    }
}

void UKOInventoryWidget::RebuildSlotWidgets()
{
    if (!SlotsPanel || !SlotWidgetClass || !InventoryComponent)
    {
        return;
    }

    const TArray<FKOItemSlot>& Slots = InventoryComponent->GetSlots();
    const int32 DesiredCount = FMath::Max(Slots.Num(), InventoryComponent->MaxSlots);

    // 부족하면 새로 생성해서 부착
    while (SlotWidgets.Num() < DesiredCount)
    {
        const int32 NewIndex = SlotWidgets.Num();
        UKOInventorySlotWidget* NewSlot = CreateWidget<UKOInventorySlotWidget>(this, SlotWidgetClass);
        if (!NewSlot)
        {
            break;
        }
        NewSlot->SetupSlot(this, NewIndex);
        SlotWidgets.Add(NewSlot);
        SlotsPanel->AddChildToWrapBox(NewSlot);
    }

    // 남으면 제거
    while (SlotWidgets.Num() > DesiredCount)
    {
        const int32 LastIndex = SlotWidgets.Num() - 1;
        if (UKOInventorySlotWidget* Tail = SlotWidgets[LastIndex])
        {
            Tail->RemoveFromParent();
        }
        SlotWidgets.RemoveAt(LastIndex);
    }

    // 데이터 갱신
    for (int32 i = 0; i < SlotWidgets.Num(); ++i)
    {
        UKOInventorySlotWidget* SlotWidget = SlotWidgets[i];
        if (!SlotWidget)
        {
            continue;
        }

        if (Slots.IsValidIndex(i))
        {
            SlotWidget->SetSlotData(Slots[i]);
        }
        else
        {
            SlotWidget->SetSlotData(FKOItemSlot{});
        }
    }
}

