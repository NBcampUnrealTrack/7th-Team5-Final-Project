// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Inventory/KOInventoryPanelWidget.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "UI/Inventory/KOInventoryWidget.h"
#include "UI/Inventory/KOEquipmentSlotWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

UKOInventoryPanelWidget::UKOInventoryPanelWidget()
{
    InputMode = EKOUIInputMode::All;

    // Back(ESC) 입력 시 자동으로 Deactivate되어 스택에서 닫힌다. (토글 제거 → Back 일원화)
    bIsBackHandler = true;
}

void UKOInventoryPanelWidget::SetInventoryComponent(UKOInventoryComponent* InComponent)
{    
    if (InventoryWidget)
    {
        InventoryWidget->SetInventoryComponent(InComponent);
    }
}

void UKOInventoryPanelWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    
    if (WarningFadeAnim)
    {
        FWidgetAnimationDynamicEvent EndEvent;
        EndEvent.BindDynamic(this, &ThisClass::OnFadeOutFinished);
        BindToAnimationFinished(WarningFadeAnim, EndEvent);
    }
}

void UKOInventoryPanelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (InventoryWidget)
    {
        InventoryWidget->OnSlotClicked.AddDynamic(this, &UKOInventoryPanelWidget::HandleSlotClicked);
        InventoryWidget->OnSlotRightClicked.AddDynamic(this, &UKOInventoryPanelWidget::HandleSlotRightClicked);
    }

    if (BuildQuickSlotBar)
    {
        BuildQuickSlotBar->SetDisplayMode(EKOQuickSlotBarDisplayMode::Inventory);
    }
    
    if (APawn* OwningPawn = GetOwningPlayerPawn())
    {
        UKOEquipmentComponent* EquipComp = OwningPawn->FindComponentByClass<UKOEquipmentComponent>();
        if (EquipComp)
        {
            EquipComp->OnEquipmentChangeBlocked.AddDynamic(this, &ThisClass::HandleEquipmentChangeBlocked);
        }
    }
    
    if (WarningText)
    {
        WarningText->SetVisibility(ESlateVisibility::Collapsed);
    }
    
    CacheEquipmentSlotWidgets();
}

void UKOInventoryPanelWidget::NativeDestruct()
{
    if (InventoryWidget)
    {
        InventoryWidget->OnSlotClicked.RemoveDynamic(this, &UKOInventoryPanelWidget::HandleSlotClicked);
        InventoryWidget->OnSlotRightClicked.RemoveDynamic(this, &UKOInventoryPanelWidget::HandleSlotRightClicked);
    }
    
    if (APawn* OwningPawn = GetOwningPlayerPawn())
    {
        UKOEquipmentComponent* EquipComp = OwningPawn->FindComponentByClass<UKOEquipmentComponent>();
        if (EquipComp)
        {
            EquipComp->OnEquipmentChangeBlocked.RemoveDynamic(this, &ThisClass::HandleEquipmentChangeBlocked);
        }
    }

    Super::NativeDestruct();
}

void UKOInventoryPanelWidget::HandleSlotClicked(int32 SlotIndex, const FKOItemSlot& InSlot)
{
    OnSlotClicked(SlotIndex, InSlot);
}

void UKOInventoryPanelWidget::HandleSlotRightClicked(int32 SlotIndex, const FKOItemSlot& InSlot)
{
    TryEquipItemToMatchingSlot(SlotIndex, InSlot);
}

void UKOInventoryPanelWidget::HandleEquipmentChangeBlocked()
{
    if (WarningText && WarningFadeAnim)
    {
        if (IsAnimationPlaying(WarningFadeAnim))
        {
            StopAnimation(WarningFadeAnim);
        }

        WarningText->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

        PlayAnimation(WarningFadeAnim);
    }
}

void UKOInventoryPanelWidget::OnFadeOutFinished()
{
    // StopAnimation()으로 중단된 이전 재생의 Finished 브로드캐스트가 다음 틱에 지연 도착할 수 있다.
    // 그 사이 새 PlayAnimation이 시작됐다면 IsAnimationPlaying()이 true이므로 여기서 Collapsed로
    // 전환하지 않아야, 방금 재시작된 애니메이션이 잘려나가지 않는다.
    if (WarningText && WarningFadeAnim && IsAnimationPlaying(WarningFadeAnim) == false)
    {
        WarningText->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void UKOInventoryPanelWidget::CacheEquipmentSlotWidgets()
{
    EquipmentSlotWidgets.Reset();

    if (!WidgetTree)
    {
        return;
    }

    WidgetTree->ForEachWidget([this](UWidget* Widget)
    {
        if (UKOEquipmentSlotWidget* EquipmentSlot = Cast<UKOEquipmentSlotWidget>(Widget))
        {
            EquipmentSlotWidgets.Add(EquipmentSlot);
        }
    });
}

bool UKOInventoryPanelWidget::TryEquipItemToMatchingSlot(int32 SlotIndex, const FKOItemSlot& InSlot)
{
    if (InSlot.Kind != EKOSlotKind::Item || InSlot.ItemId.IsNone())
    {
        return false;
    }

    for (UKOEquipmentSlotWidget* EquipmentSlot : EquipmentSlotWidgets)
    {
        if (!EquipmentSlot)
        {
            continue;
        }

        if (EquipmentSlot->TryEquipItemFromInventorySlot(SlotIndex, InSlot.ItemId))
        {
            return true;
        }
    }

    return false;
}

void UKOInventoryPanelWidget::OnSlotClicked_Implementation(int32 SlotIndex, const FKOItemSlot& InSlot)
{
    // 기본 동작 없음. 자식 클래스/BP에서 오버라이드.
}
