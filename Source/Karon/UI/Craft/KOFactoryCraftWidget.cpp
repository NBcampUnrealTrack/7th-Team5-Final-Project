#include "UI/Craft/KOFactoryCraftWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"

#include "Component/Inventory/KOInventoryComponent.h"
#include "Data/KODataTableTypes.h"
#include "Items/KOItemLibrary.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "UI/Craft/KOFactoryCraftCostEntryWidget.h"
#include "UI/Craft/KOFactoryCraftEntryWidget.h"

UKOFactoryCraftWidget::UKOFactoryCraftWidget()
{
    InputMode = EKOUIInputMode::All;
    bIsBackHandler = true;
}

void UKOFactoryCraftWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (CraftButton)
    {
        CraftButton->OnClicked.AddDynamic(this, &UKOFactoryCraftWidget::HandleCraftButtonClicked);
    }
    
    if (DecreaseCraftCountButton)
    {
        DecreaseCraftCountButton->OnClicked.AddDynamic(this, &UKOFactoryCraftWidget::HandleDecreaseCraftCountClicked);
    }

    if (IncreaseCraftCountButton)
    {
        IncreaseCraftCountButton->OnClicked.AddDynamic(this, &UKOFactoryCraftWidget::HandleIncreaseCraftCountClicked);
    }
    
    if (DecreaseCraftCount10Button)
    {
        DecreaseCraftCount10Button->OnClicked.AddDynamic(
            this, &UKOFactoryCraftWidget::HandleDecreaseCraftCount10Clicked
        );
    }

    if (IncreaseCraftCount10Button)
    {
        IncreaseCraftCount10Button->OnClicked.AddDynamic(
            this, &UKOFactoryCraftWidget::HandleIncreaseCraftCount10Clicked
        );
    }

    if (!CachedInventory.IsValid())
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            CachedInventory = PC->FindComponentByClass<UKOInventoryComponent>();
        }
    }

    CraftCount = 1;
    RefreshCraftCountText();
    Refresh();
}

void UKOFactoryCraftWidget::NativeDestruct()
{
    if (CraftButton)
    {
        CraftButton->OnClicked.RemoveDynamic(this, &UKOFactoryCraftWidget::HandleCraftButtonClicked);
    }
    
    if (DecreaseCraftCountButton)
    {
        DecreaseCraftCountButton->OnClicked.RemoveDynamic(
            this, &UKOFactoryCraftWidget::HandleDecreaseCraftCountClicked
            );
    }

    if (IncreaseCraftCountButton)
    {
        IncreaseCraftCountButton->OnClicked.RemoveDynamic(
            this, &UKOFactoryCraftWidget::HandleIncreaseCraftCountClicked
        );
    }
    
    if (DecreaseCraftCount10Button)
    {
        DecreaseCraftCount10Button->OnClicked.RemoveDynamic(
            this, &UKOFactoryCraftWidget::HandleDecreaseCraftCount10Clicked
        );
    }

    if (IncreaseCraftCount10Button)
    {
        IncreaseCraftCount10Button->OnClicked.RemoveDynamic(
            this, &UKOFactoryCraftWidget::HandleIncreaseCraftCount10Clicked
        );
    }

    Super::NativeDestruct();
}

void UKOFactoryCraftWidget::SetInventoryComponent(UKOInventoryComponent* InInventory)
{
    CachedInventory = InInventory;
    Refresh();
}

void UKOFactoryCraftWidget::Refresh()
{
    RebuildFactoryList();
    RefreshDetail();
    RefreshCraftCountText();
}

void UKOFactoryCraftWidget::RebuildFactoryList()
{
    if (!FactoryListBox)
    {
        UE_LOG(LogTemp, Warning, TEXT("[FactoryCraft] FactoryListBox가 없습니다."));
        return;
    }

    if (!EntryWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[FactoryCraft] EntryWidgetClass가 설정되지 않았습니다."));
        return;
    }

    UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    if (!LoadSub)
    {
        UE_LOG(LogTemp, Warning, TEXT("[FactoryCraft] KOLoadSubsystem을 찾을 수 없습니다."));
        return;
    }

    FactoryListBox->ClearChildren();

    FKOBuildMenuQuery Query;

    TArray<FName> FactoryIds;
    LoadSub->GetBuildableFactoryIds(Query, FactoryIds);

    for (const FName FactoryId : FactoryIds)
    {
        const FKOFactoryRow* Row = LoadSub->FindFactoryRow(FactoryId);
        if (!Row)
        {
            continue;
        }

        UKOFactoryCraftEntryWidget* EntryWidget =
            CreateWidget<UKOFactoryCraftEntryWidget>(GetOwningPlayer(), EntryWidgetClass);

        if (!EntryWidget)
        {
            continue;
        }

        UTexture2D* Icon = LoadSub->ResolveFactoryIcon(FactoryId);
        
        const bool bCanCraft = CanCraftFactory(FactoryId, 1);

        EntryWidget->SetupEntry(
            FactoryId,
            Row->DisplayName,
            Icon,
            bCanCraft
        );

        EntryWidget->OnClicked.AddDynamic(this, &UKOFactoryCraftWidget::HandleFactoryEntryClicked);

        FactoryListBox->AddChild(EntryWidget);

        if (SelectedFactoryId.IsNone())
        {
            SelectedFactoryId = FactoryId;
        }
    }
}

void UKOFactoryCraftWidget::HandleFactoryEntryClicked(FName InFactoryId)
{
    SelectedFactoryId = InFactoryId;
    CraftCount = MinCraftCount;

    RefreshCraftCountText();
    RefreshDetail();
}

void UKOFactoryCraftWidget::RefreshDetail()
{
    UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    UKOInventoryComponent* Inventory = CachedInventory.Get();

    if (!LoadSub || SelectedFactoryId.IsNone())
    {
        if (FactoryIconImage)
        {
            FactoryIconImage->SetBrushFromTexture(nullptr);
        }

        if (FactoryNameText)
        {
            FactoryNameText->SetText(FText::GetEmpty());
        }

        if (FactoryDescriptionText)
        {
            FactoryDescriptionText->SetText(FText::GetEmpty());
        }

        if (OwnedCountText)
        {
            OwnedCountText->SetText(FText::GetEmpty());
        }

        if (CostListBox)
        {
            CostListBox->ClearChildren();
        }

        RefreshCraftButtonState();
        return;
    }

    const FKOFactoryRow* Row = LoadSub->FindFactoryRow(SelectedFactoryId);
    if (!Row)
    {
        return;
    }

    if (FactoryIconImage)
    {
        FactoryIconImage->SetBrushFromTexture(LoadSub->ResolveFactoryIcon(SelectedFactoryId));
        FactoryIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }

    if (FactoryNameText)
    {
        FactoryNameText->SetText(Row->DisplayName);
    }

    if (FactoryDescriptionText)
    {
        FactoryDescriptionText->SetText(Row->Description);
    }

    if (OwnedCountText)
    {
        const int32 OwnedCount = Inventory ? Inventory->GetCountOf(SelectedFactoryId) : 0;
        OwnedCountText->SetText(FText::FromString(
            FString::Printf(TEXT("보유: %d"), OwnedCount)
        ));
    }

    RebuildCostList(Row);
    RefreshCraftButtonState();
}

void UKOFactoryCraftWidget::RebuildCostList(const FKOFactoryRow* FactoryRow)
{
    if (!CostListBox)
    {
        return;
    }

    CostListBox->ClearChildren();

    if (!FactoryRow || !CostEntryWidgetClass)
    {
        return;
    }

    UKOInventoryComponent* Inventory = CachedInventory.Get();
    if (!Inventory)
    {
        return;
    }
    
    TArray<TPair<FName, int32>> RequiredItems;
    if (!BuildRequiredItems(SelectedFactoryId, CraftCount, RequiredItems))
    {
        return;
    }

    for (const TPair<FName, int32>& RequiredItem : RequiredItems)
    {
        const FName ItemId = RequiredItem.Key;
        const int32 RequiredCount = RequiredItem.Value;
        const int32 OwnedCount = Inventory->GetCountOf(ItemId);

        UKOFactoryCraftCostEntryWidget* CostEntry =
            CreateWidget<UKOFactoryCraftCostEntryWidget>(GetOwningPlayer(), CostEntryWidgetClass);

        if (!CostEntry)
        {
            continue;
        }

        const FText ItemName = UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Item, ItemId);
        UTexture2D* ItemIcon = UKOItemLibrary::GetIcon(this, EKOSlotKind::Item, ItemId);

        CostEntry->SetupCost(
            ItemName,
            ItemIcon,
            OwnedCount,
            RequiredCount
        );

        UPanelSlot* AddedSlot = CostListBox->AddChild(CostEntry);

        if (UVerticalBoxSlot* VerticalBoxSlot = Cast<UVerticalBoxSlot>(AddedSlot))
        {
            VerticalBoxSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
        }
    }
}

bool UKOFactoryCraftWidget::CanCraftFactory(FName FactoryId, int32 InCraftCount) const
{
    return GetCraftAvailability(FactoryId, InCraftCount)
         == EKOFactoryCraftAvailability::CanCraft;
}

EKOFactoryCraftAvailability UKOFactoryCraftWidget::GetCraftAvailability(FName FactoryId, int32 InCraftCount) const
{
    const UKOInventoryComponent* Inventory = CachedInventory.Get();

    if (!Inventory || FactoryId.IsNone() || InCraftCount <= 0)
    {
        return EKOFactoryCraftAvailability::Invalid;
    }

    TArray<TPair<FName, int32>> RequiredItems; // 제작 시 제거될 아이템
    if (!BuildRequiredItems(FactoryId, InCraftCount, RequiredItems))
    {
        return EKOFactoryCraftAvailability::Invalid;
    }
    
    for (const TPair<FName, int32>& RequiredItem : RequiredItems)
    {
        if (!Inventory->HasEnoughItems(RequiredItem.Key, RequiredItem.Value))
        {
            return EKOFactoryCraftAvailability::NotEnoughMaterials;
        }
    }
    
    const int32 AddableFactoryCount = Inventory->GetAddCountAfterRemoving(
        EKOSlotKind::Factory,
        FactoryId,
        InCraftCount,
        RequiredItems
    );

    if (AddableFactoryCount < InCraftCount)
    {
        return EKOFactoryCraftAvailability::NotEnoughInventorySpace;
    }

    return EKOFactoryCraftAvailability::CanCraft;
}

void UKOFactoryCraftWidget::RefreshCraftButtonState()
{
    if (!CraftButton)
    {
        return;
    }

    const EKOFactoryCraftAvailability Availability = GetCraftAvailability(SelectedFactoryId, CraftCount);

    const bool bCanCraft = Availability == EKOFactoryCraftAvailability::CanCraft;

    CraftButton->SetIsEnabled(bCanCraft);

    CraftButton->SetBackgroundColor(
        bCanCraft
            ? CraftableButtonColor
            : NotCraftableButtonColor
    );

    if (CraftButtonText)
    {
        FText ButtonText;

        switch (Availability)
        {
        case EKOFactoryCraftAvailability::CanCraft:
            ButtonText = FText::FromString(TEXT("제작"));
            break;

        case EKOFactoryCraftAvailability::NotEnoughMaterials:
            ButtonText = FText::FromString(TEXT("재료 부족"));
            break;

        case EKOFactoryCraftAvailability::NotEnoughInventorySpace:
            ButtonText = FText::FromString(TEXT("인벤토리 공간 부족"));
            break;

        default:
            ButtonText = FText::FromString(TEXT("제작 불가"));
            break;
        }

        CraftButtonText->SetText(ButtonText);
    }
}

void UKOFactoryCraftWidget::HandleCraftButtonClicked()
{
    CraftSelectedFactory();
}

void UKOFactoryCraftWidget::HandleDecreaseCraftCountClicked()
{
    SetCraftCount(CraftCount - 1);
}

void UKOFactoryCraftWidget::HandleIncreaseCraftCountClicked()
{
    SetCraftCount(CraftCount + 1);
}

void UKOFactoryCraftWidget::HandleDecreaseCraftCount10Clicked()
{
    SetCraftCount(CraftCount - 10);
}

void UKOFactoryCraftWidget::HandleIncreaseCraftCount10Clicked()
{
    SetCraftCount(CraftCount + 10);
}

bool UKOFactoryCraftWidget::CraftSelectedFactory()
{
    UKOInventoryComponent* Inventory = CachedInventory.Get();

    if (!Inventory || SelectedFactoryId.IsNone())
    {
        return false;
    }

    if (!CanCraftFactory(SelectedFactoryId, CraftCount))
    {
        return false;
    }
    
    TArray<TPair<FName, int32>> RequiredItems;
    if (!BuildRequiredItems(SelectedFactoryId, CraftCount, RequiredItems))
    {
        return false;
    }

    // 재료 차감
    TArray<TPair<FName, int32>> RemovedItems;

    for (const TPair<FName, int32>& RequiredItem : RequiredItems)
    {
        const FName ItemId = RequiredItem.Key;
        const int32 RequiredCount = RequiredItem.Value;

        if (!Inventory->TryRemoveItem(ItemId, RequiredCount))
        {
            // 재료 차감 복구
            for (const TPair<FName, int32>& Removed : RemovedItems)
            {
                Inventory->TryAddItem(EKOSlotKind::Item, Removed.Key, Removed.Value);
            }

            return false;
        }

        RemovedItems.Add(RequiredItem);
    }

    // 설비 지급
    const int32 Remaining = Inventory->TryAddItem(EKOSlotKind::Factory, SelectedFactoryId, CraftCount);

    if (Remaining > 0)
    {
        // 재료 복구
        for (const TPair<FName, int32>& Removed : RemovedItems)
        {
            Inventory->TryAddItem(EKOSlotKind::Item, Removed.Key, Removed.Value);
        }

        UE_LOG(LogTemp, Warning, TEXT("[FactoryCraft] 설비 지급 실패: %s"), *SelectedFactoryId.ToString());
        Refresh();
        return false;
    }

    CraftCount = MinCraftCount;
    RefreshCraftCountText();
    Refresh();
    return true;
}

bool UKOFactoryCraftWidget::BuildRequiredItems(FName FactoryId, int32 InCraftCount,
    TArray<TPair<FName, int32>>& OutRequiredItems) const
{
    OutRequiredItems.Reset();

    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);

    if (!LoadSub || FactoryId.IsNone() || InCraftCount <= 0)
    {
        return false;
    }

    const FKOFactoryRow* Row = LoadSub->FindFactoryRow(FactoryId);
    if (!Row)
    {
        return false;
    }

    for (const TPair<FGameplayTag, int32>& Cost : Row->CraftCosts)
    {
        const FName ItemId = LoadSub->FindItemIdByTag(Cost.Key);
        if (ItemId.IsNone())
        {
            UE_LOG(LogTemp, Warning, TEXT("[FactoryCraft] ItemTag 해석 실패: %s"), *Cost.Key.ToString());
            return false;
        }

        const int32 RequiredPerOne = Cost.Value;
        if (RequiredPerOne <= 0)
        {
            continue;
        }

        const int32 RequiredCount = RequiredPerOne * InCraftCount;
        OutRequiredItems.Add(TPair<FName, int32>(ItemId, RequiredCount));
    }

    return true;
}

void UKOFactoryCraftWidget::SetCraftCount(int32 NewCount)
{
    const int32 MaxCraftableCount = GetMaxCraftableCount(SelectedFactoryId);

    int32 ClampedCount = MinCraftCount;

    if (MaxCraftableCount > 0)
    {
        ClampedCount = FMath::Clamp(NewCount, MinCraftCount, MaxCraftableCount);
    }

    if (CraftCount == ClampedCount)
    {
        return;
    }

    CraftCount = ClampedCount;

    RefreshCraftCountText();
    RefreshDetail();
}

void UKOFactoryCraftWidget::RefreshCraftCountText()
{
    if (CraftCountText)
    {
        CraftCountText->SetText(FText::AsNumber(CraftCount));
    }
}

int32 UKOFactoryCraftWidget::GetMaxCraftableCount(FName FactoryId) const
{
    const UKOInventoryComponent* Inventory = CachedInventory.Get();

    if (!Inventory || FactoryId.IsNone())
    {
        return 0;
    }
    
    TArray<TPair<FName, int32>> RequiredItemsPerOne;
    if (!BuildRequiredItems(FactoryId, 1, RequiredItemsPerOne))
    {
        return 0;
    }
    
    if (RequiredItemsPerOne.IsEmpty())
    {
        return 0;
    }

    int32 MaxByMaterials = TNumericLimits<int32>::Max();

    for (const TPair<FName, int32>& RequiredItem : RequiredItemsPerOne)
    {
        const FName ItemId = RequiredItem.Key;
        const int32 RequiredPerOne = RequiredItem.Value;

        if (RequiredPerOne <= 0)
        {
            continue;
        }

        const int32 OwnedCount = Inventory->GetCountOf(ItemId);
        const int32 CraftableByThisItem = OwnedCount / RequiredPerOne;

        MaxByMaterials = FMath::Min(MaxByMaterials, CraftableByThisItem);
    }

    if (MaxByMaterials == TNumericLimits<int32>::Max())
    {
        return 0;
    }

    MaxByMaterials = FMath::Max(0, MaxByMaterials);
    MaxByMaterials = FMath::Min(MaxByMaterials, MaxCraftCountLimit);

    for (int32 Count = MaxByMaterials; Count >= 1; --Count)
    {
        if (CanCraftFactory(FactoryId, Count))
        {
            return Count;
        }
    }

    return 0;
}