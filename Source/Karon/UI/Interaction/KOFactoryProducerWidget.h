// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "GMRouterSubsystem.h"
#include "KOFactoryProducerWidget.generated.h"

class AKOBaseBuilding;
class UKOEnergyProducerComponent;
class UTextBlock;
class UKOFactorySlotWidget;
class UKOInventoryWidget;

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOFactoryProducerWidget : public UKOActivatableWidget
{
    GENERATED_BODY()

public:
    UKOFactoryProducerWidget();

    UFUNCTION(BlueprintPure, Category = "KO|UI|Interaction")
    AKOBaseBuilding* GetTargetBuilding() const { return TargetBuilding.Get(); }

    UFUNCTION(BlueprintPure, Category = "KO|UI|Interaction")
    UKOEnergyProducerComponent* GetProducer() const { return Producer.Get(); }

protected:
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;

    /** 매 틱 갱신할 동적 요소(FuelBar + FuelSlot)만 처리. */
    void TickRefresh();

    /** 1회 또는 GMS 시점에만 호출되는 정적 요소(Title/PowerSpec/FuelName). */
    void RefreshStaticInfo();

    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Interaction")
    float RefreshInterval = 0.1f;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TitleText;

    /** 연료당 에너지량 표시. */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> PowerPerFuelText;

    /** 초당 에너지 생산량 표시. */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> EnergyPerSecText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UKOFactorySlotWidget> FuelSlot;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UKOInventoryWidget> InventoryWidget;

private:
    UFUNCTION()
    void HandleFuelChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload);

    TWeakObjectPtr<AKOBaseBuilding> TargetBuilding;
    TWeakObjectPtr<UKOEnergyProducerComponent> Producer;

    FTimerHandle RefreshTimerHandle;

    FGameplayMessageCallback FuelChangedCallback;
    FGameplayMessageHandle   FuelChangedHandle;
};
