// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOFactoryProducerWidget.h"

#include "Building/KOBaseBuilding.h"
#include "Component/KOEnergyProducerComponent.h"
#include "Component/KOInteractionComponent.h"
#include "GameFramework/PlayerController.h"

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

    BP_OnProducerRefreshed();
}

void UKOFactoryProducerWidget::NativeOnDeactivated()
{
    Producer.Reset();
    TargetBuilding.Reset();

    Super::NativeOnDeactivated();
}
