// Copyright Karon Team 5. All Rights Reserved.
#include "UI/KOInventorySlotWidget.h"
#include "UI/KOInventoryWidget.h"
#include "Items/KOItemLibrary.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"

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
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (UKOInventoryWidget* Owner = OwningInventory.Get())
        {
            Owner->NotifySlotClicked(SlotIndex, SlotData);
            return FReply::Handled();
        }
    }

    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}
