// Copyright Karon Team 5. All Rights Reserved.
#include "UI/KOInventoryWidget.h"
#include "Component/KOInventoryComponent.h"
#include "Messaging/KOMessageTypes.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "StructUtils/InstancedStruct.h"

void UKOInventoryWidget::SetInventoryComponent(UKOInventoryComponent* InComponent)
{
    InventoryComponent = InComponent;
    if (InventoryComponent)
    {
        RefreshSlots();
    }
}

UKOInventoryComponent* UKOInventoryWidget::GetInventoryComponent() const
{
    return InventoryComponent;
}

void UKOInventoryWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    InventoryChangedCallback.BindDynamic(this, &UKOInventoryWidget::OnInventoryChangedGMS);
    SubscribeToGMS(KOGameplayTags::Data_Message_Inventory_Changed, InventoryChangedCallback);

    RefreshSlots();
}

void UKOInventoryWidget::NativeOnDeactivated()
{
    UnsubscribeFromGMS(KOGameplayTags::Data_Message_Inventory_Changed, InventoryChangedCallback);
    InventoryChangedCallback.Clear();

    Super::NativeOnDeactivated();
}

void UKOInventoryWidget::RefreshSlots()
{
    if (!InventoryComponent)
    {
        return;
    }
    BP_OnSlotsRefreshed(InventoryComponent->GetSlots());
}

void UKOInventoryWidget::OnInventoryChangedGMS(FGameplayTag Channel, const FInstancedStruct& Payload)
{
    RefreshSlots();
}
