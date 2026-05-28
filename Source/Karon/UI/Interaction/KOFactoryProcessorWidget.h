// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOFactoryProcessorWidget.generated.h"

class AKOBaseBuilding;
class UKOFactoryProcessorComponent;
class UTextBlock;
class UProgressBar;
class UPanelWidget;
class UKOFactorySlotWidget;
class UKOInventoryWidget;

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOFactoryProcessorWidget : public UKOActivatableWidget
{
    GENERATED_BODY()

public:
    UKOFactoryProcessorWidget();

    UFUNCTION(BlueprintPure, Category = "KO|UI|Interaction")
    AKOBaseBuilding* GetTargetBuilding() const { return TargetBuilding.Get(); }

    UFUNCTION(BlueprintPure, Category = "KO|UI|Interaction")
    UKOFactoryProcessorComponent* GetProcessor() const { return Processor.Get(); }

protected:
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    
    void Refresh();
    
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Interaction")
    float RefreshInterval = 0.1f;
    
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> TitleText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> RecipeText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> StateText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> ProgressBar;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> SupplyBar;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> InputBufferText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> OutputBufferText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UPanelWidget> InputSlotsPanel;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UPanelWidget> OutputSlotsPanel;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UKOInventoryWidget> InventoryWidget;

    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Interaction")
    TSubclassOf<UKOFactorySlotWidget> InputSlotClass;

    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Interaction")
    TSubclassOf<UKOFactorySlotWidget> OutputSlotClass;

private:
    void BuildIOSlots();
    void RefreshIOSlots();

    TWeakObjectPtr<AKOBaseBuilding> TargetBuilding;
    TWeakObjectPtr<UKOFactoryProcessorComponent> Processor;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UKOFactorySlotWidget>> InputSlotWidgets;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UKOFactorySlotWidget>> OutputSlotWidgets;

    FTimerHandle RefreshTimerHandle;
};
