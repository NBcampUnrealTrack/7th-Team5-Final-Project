// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOFactoryProcessorWidget.h"

#include "Building/KOBaseBuilding.h"
#include "Component/KOFactoryProcessorComponent.h"
#include "Component/KOInteractionComponent.h"
#include "Component/KOInventoryComponent.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Data/KODataTableTypes.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "TimerManager.h"
#include "Components/Button.h"
#include "UI/Interaction/KOFactorySlotWidget.h"
#include "UI/Interaction/KOFactoryRecipeEntryWidget.h"
#include "UI/KOInventoryWidget.h"

#define LOCTEXT_NAMESPACE "KOFactoryProcessorWidget"

UKOFactoryProcessorWidget::UKOFactoryProcessorWidget()
{
    InputMode = EKOUIInputMode::All;
}

namespace
{
    FText StateToText(EKOFactoryState State)
    {
        switch (State)
        {
        case EKOFactoryState::Running:       return LOCTEXT("State_Running",       "가공 중");
        case EKOFactoryState::OutputBlocked: return LOCTEXT("State_OutputBlocked", "출력 가득");
        case EKOFactoryState::Idle:
        default:                             return LOCTEXT("State_Idle",          "대기");
        }
    }

    FString BufferToString(const UObject* WorldContext, const TMap<FName, int32>& Buffer)
    {
        if (Buffer.Num() == 0) return TEXT("-");

        TArray<FString> Parts;
        Parts.Reserve(Buffer.Num());
        for (const TPair<FName, int32>& Pair : Buffer)
        {
            if (Pair.Value <= 0) continue;
            const FText DisplayName = UKOItemLibrary::GetDisplayName(WorldContext, EKOSlotKind::Item, Pair.Key);
            const FString NameStr = DisplayName.IsEmpty() ? Pair.Key.ToString() : DisplayName.ToString();
            Parts.Add(FString::Printf(TEXT("%s x%d"), *NameStr, Pair.Value));
        }
        return Parts.Num() > 0 ? FString::Join(Parts, TEXT(", ")) : TEXT("-");
    }
}

void UKOFactoryProcessorWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    if (APlayerController* PC = GetOwningPlayer())
    {
        if (UKOInteractionComponent* IC = PC->FindComponentByClass<UKOInteractionComponent>())
        {
            TargetBuilding = Cast<AKOBaseBuilding>(IC->GetCurrentInteractable());
        }
    }

    if (!TargetBuilding.IsValid()) return;

    Processor = TargetBuilding->FindComponentByClass<UKOFactoryProcessorComponent>();
    if (!Processor.IsValid()) return;

    if (InventoryWidget)
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            UKOInventoryComponent* PlayerInv = PC->FindComponentByClass<UKOInventoryComponent>();
            if (!PlayerInv)
            {
                if (APawn* Pawn = PC->GetPawn())
                {
                    PlayerInv = Pawn->FindComponentByClass<UKOInventoryComponent>();
                }
            }
            if (PlayerInv)
            {
                InventoryWidget->SetInventoryComponent(PlayerInv);
            }
        }
    }

    BuildIOSlots();

    if (RecipeButton && !RecipeButton->OnClicked.IsAlreadyBound(this, &UKOFactoryProcessorWidget::HandleRecipeButtonClicked))
    {
        RecipeButton->OnClicked.AddDynamic(this, &UKOFactoryProcessorWidget::HandleRecipeButtonClicked);
    }
    SetRecipeSelectVisible(false);

    Refresh();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            RefreshTimerHandle,
            FTimerDelegate::CreateUObject(this, &UKOFactoryProcessorWidget::Refresh),
            RefreshInterval,
            /*bLoop=*/true);
    }
}

void UKOFactoryProcessorWidget::NativeOnDeactivated()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RefreshTimerHandle);
    }
    RefreshTimerHandle.Invalidate();

    if (InputSlotsPanel)  InputSlotsPanel->ClearChildren();
    if (OutputSlotsPanel) OutputSlotsPanel->ClearChildren();
    InputSlotWidgets.Reset();
    OutputSlotWidgets.Reset();

    if (RecipeButton)
    {
        RecipeButton->OnClicked.RemoveDynamic(this, &UKOFactoryProcessorWidget::HandleRecipeButtonClicked);
    }
    if (RecipeSelectPanel) RecipeSelectPanel->ClearChildren();
    for (UKOFactoryRecipeEntryWidget* Entry : RecipeEntryWidgets)
    {
        if (Entry)
        {
            Entry->OnRecipeClicked.RemoveDynamic(this, &UKOFactoryProcessorWidget::HandleRecipeEntryClicked);
        }
    }
    RecipeEntryWidgets.Reset();

    Processor.Reset();
    TargetBuilding.Reset();

    Super::NativeOnDeactivated();
}

void UKOFactoryProcessorWidget::HandleRecipeButtonClicked()
{
    const bool bVisible = RecipeSelectPanel && RecipeSelectPanel->GetVisibility() != ESlateVisibility::Collapsed;
    if (bVisible)
    {
        SetRecipeSelectVisible(false);
        return;
    }
    PopulateRecipeSelect();
    SetRecipeSelectVisible(true);
}

void UKOFactoryProcessorWidget::HandleRecipeEntryClicked(FName InRecipeId)
{
    if (UKOFactoryProcessorComponent* Proc = Processor.Get())
    {
        Proc->SetSelectedRecipe(InRecipeId);
    }
    SetRecipeSelectVisible(false);
    Refresh();
}

void UKOFactoryProcessorWidget::SetRecipeSelectVisible(bool bVisible)
{
    if (!RecipeSelectPanel) return;
    RecipeSelectPanel->SetVisibility(bVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UKOFactoryProcessorWidget::PopulateRecipeSelect()
{
    if (!RecipeSelectPanel || !RecipeEntryClass) return;

    AKOBaseBuilding* Building = TargetBuilding.Get();
    if (!Building) return;

    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    const FKOFactoryRow* FactoryRow = Building->GetFactoryRow();
    if (!LoadSub || !FactoryRow || !FactoryRow->FactoryCategoryTag.IsValid()) return;

    // 기존 엔트리 정리
    for (UKOFactoryRecipeEntryWidget* Entry : RecipeEntryWidgets)
    {
        if (Entry)
        {
            Entry->OnRecipeClicked.RemoveDynamic(this, &UKOFactoryProcessorWidget::HandleRecipeEntryClicked);
        }
    }
    RecipeEntryWidgets.Reset();
    RecipeSelectPanel->ClearChildren();

    auto AddEntry = [this](FName InRecipeId, const FText& InLabel)
    {
        UKOFactoryRecipeEntryWidget* Entry = CreateWidget<UKOFactoryRecipeEntryWidget>(this, RecipeEntryClass);
        if (!Entry) return;
        Entry->SetRecipe(InRecipeId, InLabel);
        Entry->OnRecipeClicked.AddDynamic(this, &UKOFactoryProcessorWidget::HandleRecipeEntryClicked);
        RecipeSelectPanel->AddChild(Entry);
        RecipeEntryWidgets.Add(Entry);
    };

    TArray<FName> AllRecipes;
    LoadSub->GetAllRecipeIds(AllRecipes);

    for (const FName& RecipeId : AllRecipes)
    {
        const FKORecipeRow* Recipe = LoadSub->FindRecipeRow(RecipeId);
        if (!Recipe || !Recipe->AllowedFactoryTag.IsValid()) continue;
        if (!FactoryRow->FactoryCategoryTag.MatchesTag(Recipe->AllowedFactoryTag)) continue;

        AddEntry(RecipeId, Recipe->DisplayName);
    }
}

void UKOFactoryProcessorWidget::BuildIOSlots()
{
    AKOBaseBuilding* Building = TargetBuilding.Get();
    UKOFactoryProcessorComponent* Proc = Processor.Get();
    if (!Building || !Proc)
    {
        return;
    }

    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    if (!LoadSub)
    {
        return;
    }

    const FKOFactoryRow* FactoryRow = Building->GetFactoryRow();
    if (!FactoryRow || !FactoryRow->FactoryCategoryTag.IsValid())
    {
        return;
    }

    // 이 공장 카테고리에 매칭되는 모든 레시피의 입력/출력 ItemId 유니온 수집.
    TArray<FName> AllRecipes;
    LoadSub->GetAllRecipeIds(AllRecipes);

    TArray<FName> InputItemIds;
    TArray<FName> OutputItemIds;

    for (const FName& RecipeId : AllRecipes)
    {
        const FKORecipeRow* Recipe = LoadSub->FindRecipeRow(RecipeId);
        if (!Recipe || !Recipe->AllowedFactoryTag.IsValid())
        {
            continue;
        }
        if (!FactoryRow->FactoryCategoryTag.MatchesTag(Recipe->AllowedFactoryTag))
        {
            continue;
        }

        for (const TPair<FGameplayTag, int32>& In : Recipe->Inputs)
        {
            const FName ItemId = LoadSub->FindItemIdByTag(In.Key);
            if (!ItemId.IsNone())
            {
                InputItemIds.AddUnique(ItemId);
            }
        }
        for (const TPair<FGameplayTag, int32>& Out : Recipe->Outputs)
        {
            const FName ItemId = LoadSub->FindItemIdByTag(Out.Key);
            if (!ItemId.IsNone())
            {
                OutputItemIds.AddUnique(ItemId);
            }
        }
    }

    if (InputSlotsPanel && InputSlotClass)
    {
        InputSlotsPanel->ClearChildren();
        InputSlotWidgets.Reset();
        for (const FName& ItemId : InputItemIds)
        {
            UKOFactorySlotWidget* SlotWidget = CreateWidget<UKOFactorySlotWidget>(this, InputSlotClass);
            if (!SlotWidget) continue;
            SlotWidget->SetupInputSlot(Proc, ItemId);
            InputSlotsPanel->AddChild(SlotWidget);
            InputSlotWidgets.Add(SlotWidget);
        }
    }

    if (OutputSlotsPanel && OutputSlotClass)
    {
        OutputSlotsPanel->ClearChildren();
        OutputSlotWidgets.Reset();
        for (const FName& ItemId : OutputItemIds)
        {
            UKOFactorySlotWidget* SlotWidget = CreateWidget<UKOFactorySlotWidget>(this, OutputSlotClass);
            if (!SlotWidget) continue;
            SlotWidget->SetupOutputSlot(Proc, ItemId);
            OutputSlotsPanel->AddChild(SlotWidget);
            OutputSlotWidgets.Add(SlotWidget);
        }
    }
}

void UKOFactoryProcessorWidget::RefreshIOSlots()
{
    for (UKOFactorySlotWidget* SlotWidget : InputSlotWidgets)
    {
        if (SlotWidget) SlotWidget->RefreshFromComponent();
    }
    for (UKOFactorySlotWidget* SlotWidget : OutputSlotWidgets)
    {
        if (SlotWidget) SlotWidget->RefreshFromComponent();
    }
}

void UKOFactoryProcessorWidget::Refresh()
{
    AKOBaseBuilding* Building = TargetBuilding.Get();
    UKOFactoryProcessorComponent* Proc = Processor.Get();
    if (!Building || !Proc) return;

    if (TitleText)
    {
        const FKOFactoryRow* Row = Building->GetFactoryRow();
        TitleText->SetText(Row ? Row->DisplayName : FText::GetEmpty());
    }

    if (RecipeText)
    {
        FText RecipeName = FText::GetEmpty();
        const FName RecipeId = Proc->GetActiveRecipeId();
        if (!RecipeId.IsNone())
        {
            if (const UKOLoadSubsystem* Load = UKOLoadSubsystem::Get(this))
            {
                if (const FKORecipeRow* Row = Load->FindRecipeRow(RecipeId))
                {
                    RecipeName = Row->DisplayName;
                }
            }
        }
        RecipeText->SetText(RecipeName);
    }

    if (StateText)
    {
        StateText->SetText(StateToText(Proc->GetState()));
    }

    if (ProgressBar)
    {
        ProgressBar->SetPercent(Proc->GetProgress());
    }

    if (SupplyBar)
    {
        SupplyBar->SetPercent(FMath::Clamp(Proc->GetLastSupplyRatio(), 0.f, 1.f));
    }

    if (InputBufferText)
    {
        InputBufferText->SetText(FText::FromString(BufferToString(this, Proc->GetInputBuffer())));
    }

    if (OutputBufferText)
    {
        OutputBufferText->SetText(FText::FromString(BufferToString(this, Proc->GetOutputBuffer())));
    }

    RefreshIOSlots();
}

#undef LOCTEXT_NAMESPACE
