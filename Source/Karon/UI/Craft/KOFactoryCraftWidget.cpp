#include "UI/Craft/KOFactoryCraftWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "CommonButtonBase.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

#include "Component/Inventory/KOInventoryComponent.h"
#include "Data/KODataTableTypes.h"
#include "Items/KOItemLibrary.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Subsystem/KOQuestGuideSubsystem.h"
#include "Subsystem/KOUnlockSubsystem.h"
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
    
    if (UKOUnlockSubsystem* UnlockSubsystem = UKOUnlockSubsystem::Get(this))
    {
        UnlockSubsystem->OnUnlockTagGranted.AddUObject(this, &UKOFactoryCraftWidget::HandleUnlockTagGranted);
    }


    if (CraftButton)
    {
        CraftButton->OnClicked().AddUObject(this, &ThisClass::HandleCraftButtonClicked);
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
        CraftButton->OnClicked().RemoveAll(this);
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
    
    if (UKOUnlockSubsystem* UnlockSubsystem = UKOUnlockSubsystem::Get(this))
    {
        UnlockSubsystem->OnUnlockTagGranted.RemoveAll(this);
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

void UKOFactoryCraftWidget::SetHideEquipmentCraftables(bool bInHide)
{
    if (bHideEquipmentCraftables == bInHide)
    {
        return;
    }

    bHideEquipmentCraftables = bInHide;
    
    SelectedTarget = FKOCraftTarget();
    CraftCount = MinCraftCount;

    RebuildFactoryList();
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

    bool bSelectedStillExists = false;

    auto AddEntry = [this, &bSelectedStillExists](const FKOCraftTarget& Target, const FText& DisplayName, UTexture2D* Icon)
    {
        UKOFactoryCraftEntryWidget* EntryWidget =
            CreateWidget<UKOFactoryCraftEntryWidget>(GetOwningPlayer(), EntryWidgetClass);

        if (!EntryWidget)
        {
            return;
        }

        const bool bCanCraft = CanCraftTarget(Target, 1);
        const UKOInventoryComponent* Inventory = CachedInventory.Get();

        const int32 OwnedCount = Inventory ? Inventory->GetCountOf(Target.Id) : 0;

        EntryWidget->SetupEntry(Target.Type, Target.Id, DisplayName, Icon, bCanCraft, OwnedCount);

        EntryWidget->OnClicked.AddDynamic(this, &UKOFactoryCraftWidget::HandleCraftEntryClicked);
        FactoryListBox->AddChild(EntryWidget);
        
        if (SelectedTarget.Type == Target.Type && SelectedTarget.Id == Target.Id)
        {
            bSelectedStillExists = true;
        }

        if (!SelectedTarget.IsValid())
        {
            SelectedTarget = Target;
        }
    };
    
    // 해금 태그 확인
    FKOBuildMenuQuery Query;

    if (const UKOUnlockSubsystem* UnlockSubsystem = UKOUnlockSubsystem::Get(this))
    {
        Query.OwnedUnlocks = UnlockSubsystem->GetOwnedUnlockTags();
    }
    else
    {
        Query.OwnedUnlocks.Reset();
    }

    // 1. 설비 목록 추가
    TArray<FName> FactoryIds;
    LoadSub->GetBuildableFactoryIds(Query, FactoryIds);

    for (const FName FactoryId : FactoryIds)
    {
        const FKOFactoryRow* Row = LoadSub->FindFactoryRow(FactoryId);
        if (!Row)
        {
            continue;
        }

        AddEntry(
            FKOCraftTarget(EKOCraftTargetType::Factory, FactoryId),
            Row->DisplayName,
            LoadSub->ResolveFactoryIcon(FactoryId)
        );
    }

    // 2. 장비 목록 추가
    TArray<FName> EquipmentIds;
    LoadSub->GetCraftableEquipmentIds(EquipmentIds);

    for (const FName EquipmentId : EquipmentIds)
    {
        const FKOEquipmentRow* Row = LoadSub->FindEquipmentRow(EquipmentId);
        if (!Row || !Row->bCraftable)
        {
            continue;
        }
        
        if (bHideEquipmentCraftables)
        {
            continue;
        }

        AddEntry(
            FKOCraftTarget(EKOCraftTargetType::Equipment, EquipmentId),
            UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Item, EquipmentId),
            UKOItemLibrary::GetIcon(this, EKOSlotKind::Item, EquipmentId)
        );
    }
    RefreshEntrySelection();
}

void UKOFactoryCraftWidget::HandleCraftEntryClicked(EKOCraftTargetType InTargetType, FName InTargetId)
{
    SelectedTarget = FKOCraftTarget(InTargetType, InTargetId);
    CraftCount = MinCraftCount;

    RefreshEntrySelection();
    RefreshCraftCountText();
    RefreshDetail();
}

void UKOFactoryCraftWidget::RefreshEntrySelection()
{
    if (!FactoryListBox)
    {
        return;
    }

    const int32 ChildCount = FactoryListBox->GetChildrenCount();

    for (int32 Index = 0; Index < ChildCount; ++Index)
    {
        UKOFactoryCraftEntryWidget* EntryWidget =
            Cast<UKOFactoryCraftEntryWidget>(FactoryListBox->GetChildAt(Index));

        if (!EntryWidget)
        {
            continue;
        }

        const bool bShouldSelect = SelectedTarget.IsValid() &&
            EntryWidget->MatchesTarget(SelectedTarget.Type, SelectedTarget.Id);

        EntryWidget->SetSelected(bShouldSelect);
    }
}

void UKOFactoryCraftWidget::RefreshDetail()
{
    UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    UKOInventoryComponent* Inventory = CachedInventory.Get();

    if (!LoadSub || !SelectedTarget.IsValid())
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

    if (SelectedTarget.Type == EKOCraftTargetType::Factory)
    {
        const FKOFactoryRow* Row = LoadSub->FindFactoryRow(SelectedTarget.Id);
        if (!Row)
        {
            return;
        }

        if (FactoryIconImage)
        {
            FactoryIconImage->SetBrushFromTexture(LoadSub->ResolveFactoryIcon(SelectedTarget.Id));
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
    }
    else
    {
        const FKOEquipmentRow* Row = LoadSub->FindEquipmentRow(SelectedTarget.Id);
        if (!Row)
        {
            return;
        }

        if (FactoryIconImage)
        {
            FactoryIconImage->SetBrushFromTexture(
                UKOItemLibrary::GetIcon(this, EKOSlotKind::Item, SelectedTarget.Id)
            );
            FactoryIconImage->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        }

        if (FactoryNameText)
        {
            FactoryNameText->SetText(
                UKOItemLibrary::GetDisplayName(this, EKOSlotKind::Item, SelectedTarget.Id)
            );
        }

        if (FactoryDescriptionText)
        {
            FText DescriptionText = FText::GetEmpty();

            const FName ItemId = LoadSub->FindItemIdByTag(Row->ItemTag);

            if (!ItemId.IsNone())
            {
                if (const FKOItemRow* ItemRow = LoadSub->FindItemRow(ItemId))
                {
                    DescriptionText = ItemRow->Description;
                }
            }

            FactoryDescriptionText->SetText(DescriptionText);
        }
    }

    if (OwnedCountText)
    {
        const int32 OwnedCount = Inventory ? Inventory->GetCountOf(SelectedTarget.Id) : 0;
        OwnedCountText->SetText(FText::FromString(
            FString::Printf(TEXT("보유: %d"), OwnedCount)
        ));
    }

    RebuildCostList();
    RefreshCraftButtonState();
}

void UKOFactoryCraftWidget::RebuildCostList()
{
    if (!CostListBox)
    {
        return;
    }

    CostListBox->ClearChildren();

    if (!CostEntryWidgetClass)
    {
        return;
    }

    UKOInventoryComponent* Inventory = CachedInventory.Get();
    if (!Inventory)
    {
        return;
    }
    
    TArray<TPair<FName, int32>> RequiredItems;
    if (!BuildRequiredItems(SelectedTarget, CraftCount, RequiredItems))
    {
        return;
    }
    
    constexpr int32 ColumnCount = 2;
    int32 AddedEntryIndex = 0;

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

        const int32 Row = AddedEntryIndex / ColumnCount;
        const int32 Column = AddedEntryIndex % ColumnCount;

        UUniformGridSlot* GridSlot = CostListBox->AddChildToUniformGrid(CostEntry, Row, Column);

        if (GridSlot)
        {
            GridSlot->SetHorizontalAlignment(HAlign_Fill);
        }

        ++AddedEntryIndex;
    }
}

bool UKOFactoryCraftWidget::CanCraftTarget(const FKOCraftTarget& Target, int32 InCraftCount) const
{
    return GetCraftAvailability(Target, InCraftCount)
        == EKOFactoryCraftAvailability::CanCraft;
}

EKOFactoryCraftAvailability UKOFactoryCraftWidget::GetCraftAvailability(const FKOCraftTarget& Target, int32 InCraftCount) const
{
    const UKOInventoryComponent* Inventory = CachedInventory.Get();

    if (!Inventory)
    {
        return EKOFactoryCraftAvailability::Invalid;
    }

    if (!Target.IsValid())
    {
        return EKOFactoryCraftAvailability::Invalid;
    }

    if (InCraftCount <= 0)
    {
        return EKOFactoryCraftAvailability::Invalid;
    }

    TArray<TPair<FName, int32>> RequiredItems; // 제작 시 제거될 아이템
    if (!BuildRequiredItems(Target, InCraftCount, RequiredItems))
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
    
    const EKOSlotKind ResultSlotKind = Target.Type == EKOCraftTargetType::Factory
            ? EKOSlotKind::Factory
            : EKOSlotKind::Item;
    
    const int32 AddableCount = Inventory->GetAddCountAfterRemoving(
        ResultSlotKind,
        Target.Id,
        InCraftCount,
        RequiredItems
    );

    if (AddableCount < InCraftCount)
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

    const EKOFactoryCraftAvailability Availability = GetCraftAvailability(SelectedTarget, CraftCount);
    const bool bCanCraft = Availability == EKOFactoryCraftAvailability::CanCraft;
    CraftButton->SetIsEnabled(bCanCraft);
    
    FText ButtonText;

    switch (Availability)
    {
    case EKOFactoryCraftAvailability::CanCraft:
        ButtonText = FText::FromString(TEXT("제작"));
        break;

    case EKOFactoryCraftAvailability::NotEnoughMaterials:
        ButtonText = FText::FromString(TEXT("재료 불가"));
        break;

    case EKOFactoryCraftAvailability::NotEnoughInventorySpace:
        ButtonText = FText::FromString(TEXT("인벤토리 공간 부족"));
        break;

    case EKOFactoryCraftAvailability::Invalid:
    default:
        ButtonText = FText::FromString(TEXT("제작 불가"));
        break;
    }

    CraftButtonText->SetText(ButtonText);
}


void UKOFactoryCraftWidget::HandleCraftButtonClicked()
{
    CraftSelectedTarget();
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

bool UKOFactoryCraftWidget::CraftSelectedTarget()
{
    UKOInventoryComponent* Inventory = CachedInventory.Get();

    if (!Inventory || !SelectedTarget.IsValid())
    {
        return false;
    }

    if (!CanCraftTarget(SelectedTarget, CraftCount))
    {
        return false;
    }
    
    TArray<TPair<FName, int32>> RequiredItems;
    if (!BuildRequiredItems(SelectedTarget, CraftCount, RequiredItems))
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

    // 설비/장비 지급
    const EKOSlotKind ResultSlotKind = SelectedTarget.Type == EKOCraftTargetType::Factory
            ? EKOSlotKind::Factory
            : EKOSlotKind::Item;

    const int32 Remaining = Inventory->TryAddItem(ResultSlotKind, SelectedTarget.Id, CraftCount);

    if (Remaining > 0)
    {
        // 재료 복구
        for (const TPair<FName, int32>& Removed : RemovedItems)
        {
            Inventory->TryAddItem(EKOSlotKind::Item, Removed.Key, Removed.Value);
        }

        UE_LOG(LogTemp, Warning, TEXT("[FactoryCraft] 설비 지급 실패: %s"), *SelectedTarget.Id.ToString());
        Refresh();
        return false;
    }
    
    // 퀘스트
    const FName CraftedItemId = SelectedTarget.Id;
    const int32 CraftedCount = CraftCount;
    
    if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
    {
        QuestGuide->NotifyItemCrafted(CraftedItemId, CraftedCount);
    }

    CraftCount = MinCraftCount;
    RefreshCraftCountText();
    Refresh();
    return true;
}

void UKOFactoryCraftWidget::HandleUnlockTagGranted(FGameplayTag GrantedTag)
{
    SelectedTarget = FKOCraftTarget();
    CraftCount = MinCraftCount;

    Refresh();
}

bool UKOFactoryCraftWidget::BuildRequiredItems(const FKOCraftTarget& Target, int32 InCraftCount, 
                                               TArray<TPair<FName, int32>>& OutRequiredItems) const
{
    OutRequiredItems.Reset();

    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);

    if (!LoadSub || !Target.IsValid() || InCraftCount <= 0)
    {
        return false;
    }

    const TMap<FGameplayTag, int32>* CraftCosts = nullptr;

    if (Target.Type == EKOCraftTargetType::Factory)
    {
        const FKOFactoryRow* Row = LoadSub->FindFactoryRow(Target.Id);
        if (!Row)
        {
            return false;
        }

        CraftCosts = &Row->CraftCosts;
    }
    else
    {
        const FKOEquipmentRow* Row = LoadSub->FindEquipmentRow(Target.Id);
        if (!Row || !Row->bCraftable)
        {
            return false;
        }

        CraftCosts = &Row->CraftCosts;
    }

    if (!CraftCosts)
    {
        return false;
    }

    for (const TPair<FGameplayTag, int32>& Cost : *CraftCosts)
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
    const int32 MaxCraftableCount = GetMaxCraftableCount(SelectedTarget);

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

int32 UKOFactoryCraftWidget::GetMaxCraftableCount(const FKOCraftTarget& Target) const
{
    const UKOInventoryComponent* Inventory = CachedInventory.Get();

    if (!Inventory || !Target.IsValid())
    {
        return 0;
    }
    
    TArray<TPair<FName, int32>> RequiredItemsPerOne;
    if (!BuildRequiredItems(Target, 1, RequiredItemsPerOne))
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
        if (CanCraftTarget(Target, Count))
        {
            return Count;
        }
    }

    return 0;
}