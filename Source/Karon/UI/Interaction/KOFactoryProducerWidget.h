// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "GMRouterSubsystem.h"
#include "KOFactoryProducerWidget.generated.h"

class AKOBaseBuilding;
class UKOEnergyProducerComponent;
class UTextBlock;
class UProgressBar;
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

    /** 연료 1단위 연소에 걸리는 시간(초) 표시. */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> CycleText;

    /** 현재 연료 1단위의 소모 진행도(0~1). Processor의 ProgressBar와 동일 패턴. */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> ProgressBar;

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
