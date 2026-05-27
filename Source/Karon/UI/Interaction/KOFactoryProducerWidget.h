// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOFactoryProducerWidget.generated.h"

class AKOBaseBuilding;
class UKOEnergyProducerComponent;
class UTextBlock;
class UProgressBar;

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
    
    void Refresh();
    
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Interaction")
    float RefreshInterval = 0.1f;
    
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TitleText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> FuelNameText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> FuelCountText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> FuelBar;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> PowerSpecText;

private:
    TWeakObjectPtr<AKOBaseBuilding> TargetBuilding;
    TWeakObjectPtr<UKOEnergyProducerComponent> Producer;

    FTimerHandle RefreshTimerHandle;
};
