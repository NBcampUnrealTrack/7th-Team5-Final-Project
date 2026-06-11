// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOBeltConnectEntryWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UKOBeltConnectEntryWidget::SetupSlot(const FKOFactoryPortSlot& InSlot, int32 SlotNumber, const FText& InDisplayName, UTexture2D* InIcon, bool bOccupied)
{
    PortSlot = InSlot;

    if (NameText)
    {
        NameText->SetText(InDisplayName);
    }
    if (SlotNumberText)
    {
        SlotNumberText->SetText(FText::AsNumber(SlotNumber));
    }
    if (IconImage)
    {
        // 아이템 힌트가 있으면 그 아이콘, 없으면(빈 포트) EmptySlotIcon, 둘 다 없으면 숨김.
        UTexture2D* Brush = InIcon ? InIcon : EmptySlotIcon.Get();
        if (Brush)
        {
            IconImage->SetBrushFromTexture(Brush);
            IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
        else
        {
            IconImage->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    // 일반 포트 모델: 빈 포트도 바인딩 가능 → 점유된 경우에만 비활성/흐리게.
    if (SlotButton)
    {
        SlotButton->SetIsEnabled(!bOccupied);
    }
    SetRenderOpacity(bOccupied ? 0.4f : 1.0f);
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
