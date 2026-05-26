// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOFactoryProcessorWidget.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Building/KOBaseBuilding.h"
#include "Component/KOFactoryProcessorComponent.h"
#include "Component/KOInteractionComponent.h"
#include "GameFramework/PlayerController.h"
#include "Messaging/KOMessageTypes.h"
#include "StructUtils/InstancedStruct.h"

void UKOFactoryProcessorWidget::NativeOnActivated()
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

    Processor = TargetBuilding->FindComponentByClass<UKOFactoryProcessorComponent>();
    if (!Processor.IsValid()) return;

    FactoryStateCallback.BindDynamic(this, &UKOFactoryProcessorWidget::OnFactoryStateChangedGMS);
    FactoryStateHandle = Subscribe(KOGameplayTags::Data_Message_Factory_StateChanged, FactoryStateCallback);

    BP_OnProcessorRefreshed();
}

void UKOFactoryProcessorWidget::NativeOnDeactivated()
{
    if (FactoryStateHandle.IsValid())
    {
        Unsubscribe(FactoryStateHandle);
        FactoryStateHandle = FGameplayMessageHandle();
    }
    FactoryStateCallback.Clear();

    Processor.Reset();
    TargetBuilding.Reset();

    Super::NativeOnDeactivated();
}

void UKOFactoryProcessorWidget::OnFactoryStateChangedGMS(FGameplayTag /*Channel*/, const FInstancedStruct& Payload)
{
    const FKOFactoryStateChangedMessage* Msg = Payload.GetPtr<FKOFactoryStateChangedMessage>();
    if (!Msg) return;

    AKOBaseBuilding* Building = TargetBuilding.Get();
    if (!Building || Msg->FactoryId != Building->GetFactoryId())
    {
        return;
    }

    BP_OnProcessorRefreshed();
}
