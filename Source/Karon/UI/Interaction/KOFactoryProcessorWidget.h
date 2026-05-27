// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOFactoryProcessorWidget.generated.h"

class AKOBaseBuilding;
class UKOFactoryProcessorComponent;
class UTextBlock;
class UProgressBar;

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

private:
    TWeakObjectPtr<AKOBaseBuilding> TargetBuilding;
    TWeakObjectPtr<UKOFactoryProcessorComponent> Processor;

    FTimerHandle RefreshTimerHandle;
};
