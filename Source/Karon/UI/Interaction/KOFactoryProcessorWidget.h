// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "GMRouterSubsystem.h"
#include "KOFactoryProcessorWidget.generated.h"

class AKOBaseBuilding;
class UKOFactoryProcessorComponent;
class UTextBlock;
class UProgressBar;
class UPanelWidget;
class UButton;
class UWidgetSwitcher;
class UKOFactorySlotWidget;
class UKOInventoryWidget;
class UKOFactoryRecipeEntryWidget;

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

    /** ProgressBar/SupplyBar처럼 매 틱 변하는 요소 전용. */
    void TickRefresh();

    /** Title 1회 세팅용. */
    void RefreshStaticInfo();

    /** Recipe/State/Buffer/IOSlots 등 이벤트 기반 요소 갱신. */
    void RefreshEventDriven();
    
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

    /** 이 가공기의 초당 전력 사용량(사용/요구). */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> PowerUseText;

    /** 이 가공기가 속한 전력망(Union)의 초당 총 생산량. */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> NetworkProductionText;

    /**
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> InputBufferText;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> OutputBufferText;
    */

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

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> RecipeButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UPanelWidget> RecipeSelectPanel;

    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Interaction")
    TSubclassOf<UKOFactoryRecipeEntryWidget> RecipeEntryClass;

    /** Inventory(0) / Recipe(1) 두 자식을 가진 WidgetSwitcher. WBP에 같은 이름으로 배치. */
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UWidgetSwitcher> PanelSwitcher;

    /** PanelSwitcher의 Inventory 페이지 인덱스. */
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Interaction")
    int32 InventoryPanelIndex = 0;

    /** PanelSwitcher의 Recipe 페이지 인덱스. */
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Interaction")
    int32 RecipePanelIndex = 1;

private:
    void BuildIOSlots();
    void RefreshIOSlots();

    void PopulateRecipeSelect();

    /** bShowingRecipePanel 상태에 맞춰 PanelSwitcher의 ActiveWidgetIndex 갱신. */
    void ApplyPanelSwitch();

    UFUNCTION()
    void HandleRecipeButtonClicked();

    UFUNCTION()
    void HandleRecipeEntryClicked(FName InRecipeId);

    UFUNCTION()
    void HandleProcessorChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload);

    TWeakObjectPtr<AKOBaseBuilding> TargetBuilding;
    TWeakObjectPtr<UKOFactoryProcessorComponent> Processor;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UKOFactorySlotWidget>> InputSlotWidgets;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UKOFactorySlotWidget>> OutputSlotWidgets;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UKOFactoryRecipeEntryWidget>> RecipeEntryWidgets;

    FTimerHandle RefreshTimerHandle;

    FGameplayMessageCallback ProcessorChangedCallback;
    FGameplayMessageHandle   ProcessorChangedHandle;

    /** true면 Recipe 패널을 보여주고 Inventory를 숨김. false면 그 반대. 기본 Inventory. */
    bool bShowingRecipePanel = false;
};
