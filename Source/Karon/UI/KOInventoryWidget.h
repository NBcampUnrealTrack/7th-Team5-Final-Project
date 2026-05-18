// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "Items/KOItemTypes.h"
#include "KHS_GMRouterManager.h"
#include "KOInventoryWidget.generated.h"

class UKOInventoryComponent;

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOInventoryWidget : public UKOActivatableWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "KO|UI|Inventory")
    void SetInventoryComponent(UKOInventoryComponent* InComponent);

    UFUNCTION(BlueprintPure, Category = "KO|UI|Inventory")
    UKOInventoryComponent* GetInventoryComponent() const;

protected:
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;

    UFUNCTION(BlueprintCallable, Category = "KO|UI|Inventory")
    void RefreshSlots();

    UFUNCTION(BlueprintImplementableEvent, Category = "KO|UI|Inventory")
    void BP_OnSlotsRefreshed(const TArray<FKOItemSlot>& Slots);

    UPROPERTY(BlueprintReadOnly, Category = "KO|UI|Inventory")
    TObjectPtr<UKOInventoryComponent> InventoryComponent;

private:
    UFUNCTION()
    void OnInventoryChangedGMS(FGameplayTag Channel, const FInstancedStruct& Payload);

    FGameplayMessageCallback InventoryChangedCallback;
};
