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

    if (!CachedInventory.IsValid())
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            CachedInventory = PC->FindComponentByClass<UKOInventoryComponent>();
        }
    }

    Refresh();
}

void UKOFactoryCraftWidget::NativeDestruct()
{
    if (CraftButton)
    {
        CraftButton->OnClicked.RemoveDynamic(this, &UKOFactoryCraftWidget::HandleCraftButtonClicked);
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

        EntryWidget->SetupEntry(
            FactoryId,
            Row->DisplayName,
            Icon
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

    if (!FactoryRow)
    {
        return;
    }

    if (!CostEntryWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[FactoryCraft] CostEntryWidgetClass가 설정되지 않았습니다."));
        return;
    }

    UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    UKOInventoryComponent* Inventory = CachedInventory.Get();

    if (!LoadSub || !Inventory)
    {
        return;
    }

    for (const TPair<FGameplayTag, int32>& Cost : FactoryRow->CraftCosts)
    {
        const FName ItemId = LoadSub->FindItemIdByTag(Cost.Key);
        if (ItemId.IsNone())
        {
            UE_LOG(LogTemp, Warning, TEXT("[FactoryCraft] ItemTag 해석 실패: %s"), *Cost.Key.ToString());
            continue;
        }

        const int32 RequiredCount = Cost.Value;
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

bool UKOFactoryCraftWidget::CanCraftFactory(FName FactoryId) const
{
    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    const UKOInventoryComponent* Inventory = CachedInventory.Get();

    if (!LoadSub || !Inventory || FactoryId.IsNone())
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
            return false;
        }

        if (!Inventory->HasEnoughItems(ItemId, Cost.Value))
        {
            return false;
        }
    }

    return true;
}

void UKOFactoryCraftWidget::RefreshCraftButtonState()
{
    if (!CraftButton)
    {
        return;
    }

    CraftButton->SetIsEnabled(CanCraftFactory(SelectedFactoryId));
}

void UKOFactoryCraftWidget::HandleCraftButtonClicked()
{
    CraftSelectedFactory();
}

bool UKOFactoryCraftWidget::CraftSelectedFactory()
{
    UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    UKOInventoryComponent* Inventory = CachedInventory.Get();

    if (!LoadSub || !Inventory || SelectedFactoryId.IsNone())
    {
        return false;
    }

    const FKOFactoryRow* Row = LoadSub->FindFactoryRow(SelectedFactoryId);
    if (!Row)
    {
        return false;
    }

    if (!CanCraftFactory(SelectedFactoryId))
    {
        return false;
    }

    // 재료 차감
    TArray<TPair<FName, int32>> RemovedItems;

    for (const TPair<FGameplayTag, int32>& Cost : Row->CraftCosts)
    {
        const FName ItemId = LoadSub->FindItemIdByTag(Cost.Key);
        if (ItemId.IsNone())
        {
            return false;
        }

        if (!Inventory->TryRemoveItem(ItemId, Cost.Value))
        {
            // 재료 차감 복구
            for (const TPair<FName, int32>& Removed : RemovedItems)
            {
                Inventory->TryAddItem(EKOSlotKind::Item, Removed.Key, Removed.Value);
            }

            return false;
        }

        RemovedItems.Add(TPair<FName, int32>(ItemId, Cost.Value));
    }

    // 설비 지급
    const int32 Remaining = Inventory->TryAddItem(EKOSlotKind::Factory, SelectedFactoryId, 1);

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

    Refresh();
    return true;
}