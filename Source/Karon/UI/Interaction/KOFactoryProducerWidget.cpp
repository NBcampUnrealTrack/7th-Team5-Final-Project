// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOFactoryProducerWidget.h"

#include "Building/KOBaseBuilding.h"
#include "Component/KOEnergyProducerComponent.h"
#include "Component/KOInteractionComponent.h"
#include "Component/KOInventoryComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Data/KODataTableTypes.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
#include "TimerManager.h"
#include "UI/Interaction/KOFactorySlotWidget.h"
#include "UI/KOInventoryWidget.h"

#define LOCTEXT_NAMESPACE "KOFactoryProducerWidget"

UKOFactoryProducerWidget::UKOFactoryProducerWidget()
{
    InputMode = EKOUIInputMode::All;
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

    Refresh();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            RefreshTimerHandle,
            FTimerDelegate::CreateUObject(this, &UKOFactoryProducerWidget::Refresh),
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

    Producer.Reset();
    TargetBuilding.Reset();

    Super::NativeOnDeactivated();
}

void UKOFactoryProducerWidget::Refresh()
{
    AKOBaseBuilding* Building = TargetBuilding.Get();
    UKOEnergyProducerComponent* Prod = Producer.Get();
    if (!Building || !Prod) return;

    if (TitleText)
    {
        const FKOFactoryRow* Row = Building->GetFactoryRow();
        TitleText->SetText(Row ? Row->DisplayName : FText::GetEmpty());
    }

    if (FuelNameText)
    {
        FText FuelName = FText::GetEmpty();
        if (Prod->FuelCategoryTag.IsValid())
        {
            FuelName = FText::FromName(Prod->FuelCategoryTag.GetTagName());
        }
        FuelNameText->SetText(FuelName);
    }

    if (FuelCountText)
    {
        const FText CountText = FText::Format(
            LOCTEXT("FuelCountFormat", "{0} / {1}"),
            FText::AsNumber(Prod->GetFuelCount()),
            FText::AsNumber(Prod->MaxFuelBuffer));
        FuelCountText->SetText(CountText);
    }

    if (FuelBar)
    {
        const float Ratio = Prod->MaxFuelBuffer > 0
            ? static_cast<float>(Prod->GetFuelCount()) / static_cast<float>(Prod->MaxFuelBuffer)
            : 0.f;
        FuelBar->SetPercent(FMath::Clamp(Ratio, 0.f, 1.f));
    }

    if (PowerSpecText)
    {
        const FText Spec = FText::Format(
            LOCTEXT("PowerSpecFormat", "{0}/연료, {1}/s"),
            FText::AsNumber(Prod->PowerPerFuelUnit),
            FText::AsNumber(Prod->BurnRatePerSecond));
        PowerSpecText->SetText(Spec);
    }

    if (FuelSlot)
    {
        FuelSlot->RefreshFromComponent();
    }
}

#undef LOCTEXT_NAMESPACE
