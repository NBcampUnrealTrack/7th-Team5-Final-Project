#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOFactoryCraftWidget.generated.h"

class UButton;
class UImage;
class UPanelWidget;
class UTextBlock;
class UKOInventoryComponent;
class UKOFactoryCraftEntryWidget;
class UKOFactoryCraftCostEntryWidget;
struct FKOFactoryRow;

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOFactoryCraftWidget : public UKOActivatableWidget
{
    GENERATED_BODY()

public:
    UKOFactoryCraftWidget();

    void SetInventoryComponent(UKOInventoryComponent* InInventory);

    UFUNCTION(BlueprintCallable, Category = "KO|FactoryCraft")
    void Refresh();

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|FactoryCraft")
    TSubclassOf<UKOFactoryCraftEntryWidget> EntryWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|FactoryCraft")
    TSubclassOf<UKOFactoryCraftCostEntryWidget> CostEntryWidgetClass;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPanelWidget> FactoryListBox;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UPanelWidget> CostListBox;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> FactoryIconImage; 

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> FactoryNameText; 

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> FactoryDescriptionText; 

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> OwnedCountText; 

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UButton> CraftButton; 
    
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> CraftButtonText;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|FactoryCraft|Style")
    FLinearColor CraftableButtonColor = FLinearColor::White;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|FactoryCraft|Style")
    FLinearColor NotCraftableButtonColor = FLinearColor(0.3f, 0.01f, 0.01f, 0.3f);

private:
    UPROPERTY()
    FName SelectedFactoryId = NAME_None;

    UPROPERTY()
    TWeakObjectPtr<UKOInventoryComponent> CachedInventory;

private:
    void RebuildFactoryList(); // 설비 목록 만듦 (왼쪽)
    void RefreshDetail(); // 선택된 설비의 상세 정보 영역을 갱신 (오른쪽)
    void RebuildCostList(const FKOFactoryRow* FactoryRow); // 선택된 설비의 필요 재료 목록을 만듦
    void RefreshCraftButtonState();

    bool CanCraftFactory(FName FactoryId) const; // 제작 가능 여부 검사
    bool CraftSelectedFactory(); // 설비 제작

    UFUNCTION()
    void HandleFactoryEntryClicked(FName InFactoryId);

    UFUNCTION()
    void HandleCraftButtonClicked();
};