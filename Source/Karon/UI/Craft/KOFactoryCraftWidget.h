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

UENUM()
enum class EKOFactoryCraftAvailability : uint8
{
    CanCraft,                   // 제작
    NotEnoughMaterials,         // 재료 부족
    NotEnoughInventorySpace,    // 인벤토리 공간 부족
    Invalid
};

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
    
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> DecreaseCraftCountButton; // -1

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> IncreaseCraftCountButton; // +1
    
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> DecreaseCraftCount10Button; // -10
    
    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> IncreaseCraftCount10Button; // +10

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> CraftCountText;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|FactoryCraft|Style")
    FLinearColor CraftableButtonColor = FLinearColor(0.71875f, 0.71875f, 0.71875f, 0.9f);
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|FactoryCraft|Style")
    FLinearColor NotCraftableButtonColor = FLinearColor(0.3f, 0.01f, 0.01f, 0.3f);

private:
    UPROPERTY()
    FName SelectedFactoryId = NAME_None;

    UPROPERTY()
    TWeakObjectPtr<UKOInventoryComponent> CachedInventory;
    
    UPROPERTY()
    int32 CraftCount = 1;
    
    UPROPERTY()
    int32 MinCraftCount = 1;
    
    UPROPERTY()
    int32 MaxCraftCountLimit = 999;

private:
    void RebuildFactoryList(); // 설비 목록 만듦 (왼쪽)
    void RefreshDetail(); // 선택된 설비의 상세 정보 영역을 갱신 (오른쪽)
    void RebuildCostList(const FKOFactoryRow* FactoryRow); // 선택된 설비의 필요 재료 목록을 만듦
    void RefreshCraftButtonState();

    // 제작 가능 여부 검사
    bool CanCraftFactory(FName FactoryId, int32 InCraftCount) const;
    EKOFactoryCraftAvailability GetCraftAvailability(FName FactoryId, int32 InCraftCount) const;
    bool BuildRequiredItems(FName FactoryId, int32 InCraftCount, TArray<TPair<FName, int32>>& OutRequiredItems) const;
    
    bool CraftSelectedFactory(); // 설비 제작
    
    void SetCraftCount(int32 NewCount);
    void RefreshCraftCountText();
    int32 GetMaxCraftableCount(FName FactoryId) const;

    UFUNCTION()
    void HandleFactoryEntryClicked(FName InFactoryId);

    UFUNCTION()
    void HandleCraftButtonClicked();
    
    UFUNCTION()
    void HandleDecreaseCraftCountClicked();

    UFUNCTION()
    void HandleIncreaseCraftCountClicked();
    
    UFUNCTION()
    void HandleDecreaseCraftCount10Clicked();

    UFUNCTION()
    void HandleIncreaseCraftCount10Clicked();
};