// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOFactoryProducerWidget.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Building/KOBaseBuilding.h"
#include "Component/Factory/KOEnergyProducerComponent.h"
#include "Component/Interaction/KOInteractionComponent.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Components/TextBlock.h"
#include "Data/KODataTableTypes.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"

#include "StructUtils/InstancedStruct.h"
#include "TimerManager.h"
#include "UI/Interaction/KOFactorySlotWidget.h"
#include "UI/Inventory/KOInventoryWidget.h"
#include "Utility/Messaging/KOMessageTypes.h"

#define LOCTEXT_NAMESPACE "KOFactoryProducerWidget"

UKOFactoryProducerWidget::UKOFactoryProducerWidget()
{
    InputMode = EKOUIInputMode::Menu;

    // Back(ESC) 입력 시 자동으로 Deactivate되어 닫힌다. (토글/재상호작용 제거 → Back 일원화)
    bIsBackHandler = true;
}

void UKOFactoryProducerWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UKOInteractionComponent* IC = PC->FindComponentByClass<UKOInteractionComponent>())
        {
            TargetBuilding = Cast<AKOBaseBuilding>(IC->GetCurrentInteractable());
        }
    }

    if (!TargetBuilding.IsValid()) return;

    Producer = TargetBuilding->FindComponentByClass<UKOEnergyProducerComponent>();
    if (!Producer.IsValid()) return;

    if (FuelSlot)
    {
        FuelSlot->SetupFuelSlot(Producer.Get());
    }

    if (InventoryWidget)
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            UKOInventoryComponent* PlayerInv = PC->FindComponentByClass<UKOInventoryComponent>();
            if (!PlayerInv)
            {
                if (APawn* Pawn = PC->GetPawn())
                {
                    PlayerInv = Pawn->FindComponentByClass<UKOInventoryComponent>();
                }
            }
            if (PlayerInv)
            {
                InventoryWidget->SetInventoryComponent(PlayerInv);
            }
        }
    }

    // GMS 구독: 연료 변경 시 FuelNameText 갱신
    FuelChangedCallback.BindDynamic(this, &UKOFactoryProducerWidget::HandleFuelChangedMessage);
    FuelChangedHandle = Subscribe(KOGameplayTags::Data_Message_Producer_FuelChanged, FuelChangedCallback);

    // 1회만 세팅하면 충분한 정적 정보
    RefreshStaticInfo();

    // 동적 요소(바 + 슬롯)만 주기적 갱신
    TickRefresh();
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            RefreshTimerHandle,
            FTimerDelegate::CreateUObject(this, &UKOFactoryProducerWidget::TickRefresh),
            RefreshInterval,
            /*bLoop=*/true);
    }
}

void UKOFactoryProducerWidget::NativeOnDeactivated()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RefreshTimerHandle);
    }
    RefreshTimerHandle.Invalidate();

    Unsubscribe(FuelChangedHandle);
    FuelChangedHandle = FGameplayMessageHandle();
    FuelChangedCallback.Clear();

    Producer.Reset();
    TargetBuilding.Reset();

    Super::NativeOnDeactivated();
}

void UKOFactoryProducerWidget::RefreshStaticInfo()
{
    AKOBaseBuilding* Building = TargetBuilding.Get();
    UKOEnergyProducerComponent* Prod = Producer.Get();
    if (!Building || !Prod) return;

    if (TitleText)
    {
        const FKOFactoryRow* Row = Building->GetFactoryRow();
        TitleText->SetText(Row ? Row->DisplayName : FText::GetEmpty());
    }

    if (PowerPerFuelText)
    {
        PowerPerFuelText->SetText(FText::Format(
            LOCTEXT("PowerPerFuelFormat", "연료당 압력 : {0}"),
            FText::AsNumber(FMath::RoundToInt(Prod->GetPowerPerFuelUnit()))));
    }

    if (EnergyPerSecText)
    {
        const float MaxPerSec = Prod->GetBurnRatePerSecond() * Prod->GetPowerPerFuelUnit();
        EnergyPerSecText->SetText(FText::Format(
            LOCTEXT("EnergyPerSecFormat", "에너지 생산 : {0}/s"),
            FText::AsNumber(FMath::RoundToInt(MaxPerSec))));
    }
}

void UKOFactoryProducerWidget::TickRefresh()
{
    UKOEnergyProducerComponent* Prod = Producer.Get();
    if (!Prod) return;

    if (FuelSlot)
    {
        FuelSlot->RefreshFromComponent();
    }
}

void UKOFactoryProducerWidget::HandleFuelChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload)
{
    const FKOProducerFuelChangedMessage* Msg = Payload.GetPtr<FKOProducerFuelChangedMessage>();
    if (!Msg) return;
    if (Msg->Producer.Get() != Producer.Get()) return;
}

#undef LOCTEXT_NAMESPACE
