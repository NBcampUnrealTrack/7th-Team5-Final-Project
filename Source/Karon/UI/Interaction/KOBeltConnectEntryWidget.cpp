// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOBeltConnectEntryWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UKOBeltConnectEntryWidget::SetupSlot(const FKOFactoryPortSlot& InSlot, const FText& InDisplayName, UTexture2D* InIcon)
{
    PortSlot = InSlot;

    if (NameText)
    {
        NameText->SetText(InDisplayName);
    }
    if (IconImage)
    {
        IconImage->SetBrushFromTexture(InIcon);
    }
}

void UKOBeltConnectEntryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (SlotButton && !SlotButton->OnClicked.IsAlreadyBound(this, &UKOBeltConnectEntryWidget::HandleButtonClicked))
    {
        SlotButton->OnClicked.AddDynamic(this, &UKOBeltConnectEntryWidget::HandleButtonClicked);
    }
}

void UKOBeltConnectEntryWidget::NativeDestruct()
{
    if (SlotButton)
    {
        SlotButton->OnClicked.RemoveDynamic(this, &UKOBeltConnectEntryWidget::HandleButtonClicked);
    }
    Super::NativeDestruct();
}

void UKOBeltConnectEntryWidget::HandleButtonClicked()
{
    OnSlotClicked.Broadcast(PortSlot);
}
