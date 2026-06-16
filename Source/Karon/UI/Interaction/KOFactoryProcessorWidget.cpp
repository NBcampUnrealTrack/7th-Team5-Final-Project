// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOFactoryProcessorWidget.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Building/KOBaseBuilding.h"
#include "Component/Factory/KOFactoryProcessorComponent.h"
#include "Component/Interaction/KOInteractionComponent.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Components/PanelWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Data/KODataTableTypes.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"

#include "StructUtils/InstancedStruct.h"
#include "Subsystem/KOEnergySubsystem.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "TimerManager.h"
#include "Components/Button.h"
#include "UI/Interaction/KOFactorySlotWidget.h"
#include "UI/Interaction/KOFactoryRecipeEntryWidget.h"
#include "UI/Inventory/KOInventoryWidget.h"
#include "Utility/Messaging/KOMessageTypes.h"

#define LOCTEXT_NAMESPACE "KOFactoryProcessorWidget"

UKOFactoryProcessorWidget::UKOFactoryProcessorWidget()
{
    InputMode = EKOUIInputMode::Menu;

    // Back(ESC) 입력 시 자동으로 Deactivate되어 닫힌다. (토글/재상호작용 제거 → Back 일원화)
    bIsBackHandler = true;
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

    bShowingRecipePanel = false;
    ApplyPanelSwitch();

    // GMS 구독: Processor의 Recipe/State/Buffer 변동 이벤트
    ProcessorChangedCallback.BindDynamic(this, &UKOFactoryProcessorWidget::HandleProcessorChangedMessage);
    ProcessorChangedHandle = Subscribe(KOGameplayTags::Data_Message_Processor_Changed, ProcessorChangedCallback);

    RefreshStaticInfo();
    RefreshEventDriven();
    TickRefresh();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            RefreshTimerHandle,
            FTimerDelegate::CreateUObject(this, &UKOFactoryProcessorWidget::TickRefresh),
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

    Unsubscribe(ProcessorChangedHandle);
    ProcessorChangedHandle = FGameplayMessageHandle();
    ProcessorChangedCallback.Clear();

    Processor.Reset();
    TargetBuilding.Reset();

    Super::NativeOnDeactivated();
}

void UKOFactoryProcessorWidget::HandleRecipeButtonClicked()
{
    // RecipeButton -> Recipe 패널 진입. Inventory는 숨김.
    bShowingRecipePanel = true;
    ApplyPanelSwitch();
}

void UKOFactoryProcessorWidget::HandleRecipeEntryClicked(FName InRecipeId)
{
    UKOFactoryProcessorComponent* Proc = Processor.Get();
    if (!Proc)
    {
        return;
    }
    
    if (Proc->CanChangeRecipe())
    {
        Proc->SetSelectedRecipe(InRecipeId);
        // SetSelectedRecipe 내부에서 BroadcastProcessorChanged → HandleProcessorChangedMessage가 UI 갱신.

        BuildIOSlots();
    }

    // 레시피 선택 후 Inventory 패널로 복귀
    bShowingRecipePanel = false;
    ApplyPanelSwitch();
}

void UKOFactoryProcessorWidget::ApplyPanelSwitch()
{
    if (bShowingRecipePanel)
    {
        PopulateRecipeSelect();
    }

    if (PanelSwitcher)
    {
        PanelSwitcher->SetActiveWidgetIndex(bShowingRecipePanel ? RecipePanelIndex : InventoryPanelIndex);
    }
}

void UKOFactoryProcessorWidget::PopulateRecipeSelect()
{
    if (!RecipeSelectPanel || !RecipeEntryClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Processor] PopulateRecipeSelect early-out: Panel=%s, EntryClass=%s"),
            RecipeSelectPanel ? TEXT("OK") : TEXT("NULL"),
            RecipeEntryClass ? TEXT("OK") : TEXT("NULL"));
        return;
    }

    AKOBaseBuilding* Building = TargetBuilding.Get();
    if (!Building) return;

    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    const FKOFactoryRow* FactoryRow = Building->GetFactoryRow();
    if (!LoadSub || !FactoryRow || !FactoryRow->FactoryCategoryTag.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("[Processor] PopulateRecipeSelect early-out: LoadSub=%s, Row=%s, Tag=%s"),
            LoadSub ? TEXT("OK") : TEXT("NULL"),
            FactoryRow ? TEXT("OK") : TEXT("NULL"),
            (FactoryRow && FactoryRow->FactoryCategoryTag.IsValid()) ? *FactoryRow->FactoryCategoryTag.ToString() : TEXT("INVALID"));
        return;
    }

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

    UE_LOG(LogTemp, Warning, TEXT("[Processor] PopulateRecipeSelect: %d entries added (FactoryTag=%s)"),
        RecipeEntryWidgets.Num(), *FactoryRow->FactoryCategoryTag.ToString());
}

void UKOFactoryProcessorWidget::BuildIOSlots()
{
    UKOFactoryProcessorComponent* Proc = Processor.Get();
    if (!Proc)
    {
        return;
    }

    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    if (!LoadSub)
    {
        return;
    }

    const FName SelectedRecipeId = Proc->GetSelectedRecipe();

    // 아직 레시피를 선택하지 않았으면 슬롯을 만들지 않음
    //if (SelectedRecipeId.IsNone()) return;

    const FKORecipeRow* Recipe = LoadSub->FindRecipeRow(SelectedRecipeId);
    if (!Recipe)
    {
        return;
    }

    // 선택된 레시피의 Input 슬롯만 생성
    if (InputSlotsPanel && InputSlotClass)
    {
        InputSlotsPanel->ClearChildren();
        InputSlotWidgets.Reset();
        
        for (const TPair<FGameplayTag, int32>& In : Recipe->Inputs)
        {
            const FName ItemId = LoadSub->FindItemIdByTag(In.Key);
            if (ItemId.IsNone()) continue;
            
            UKOFactorySlotWidget* SlotWidget = CreateWidget<UKOFactorySlotWidget>(this, InputSlotClass);
            if (!SlotWidget) continue;
            
            SlotWidget->SetupInputSlot(Proc, ItemId);
            InputSlotsPanel->AddChild(SlotWidget);
            InputSlotWidgets.Add(SlotWidget);
        }
    }

    // 선택된 레시피의 Output 슬롯만 생성
    if (OutputSlotsPanel && OutputSlotClass)
    {
        OutputSlotsPanel->ClearChildren();
        OutputSlotWidgets.Reset();
        
        for (const TPair<FGameplayTag, int32>& Out : Recipe->Outputs)
        {
            const FName ItemId = LoadSub->FindItemIdByTag(Out.Key);
            if (ItemId.IsNone()) continue;
            
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

void UKOFactoryProcessorWidget::RefreshStaticInfo()
{
    AKOBaseBuilding* Building = TargetBuilding.Get();
    if (!Building) return;

    if (TitleText)
    {
        const FKOFactoryRow* Row = Building->GetFactoryRow();
        TitleText->SetText(Row ? Row->DisplayName : FText::GetEmpty());
    }
}

void UKOFactoryProcessorWidget::TickRefresh()
{
    UKOFactoryProcessorComponent* Proc = Processor.Get();
    if (!Proc) return;

    if (ProgressBar)
    {
        ProgressBar->SetPercent(Proc->GetProgress());
    }

    if (SupplyBar)
    {
        SupplyBar->SetPercent(FMath::Clamp(Proc->GetLastSupplyRatio(), 0.f, 1.f));
    }

    if (PowerUseText)
    {
        // 사용 / 요구 (초당). 전력 부족 시 사용 < 요구.
        PowerUseText->SetText(FText::Format(
            LOCTEXT("PowerUseFormat", "{0} / {1} /s"),
            FText::AsNumber(FMath::RoundToInt(Proc->GetSuppliedPowerPerSecond())),
            FText::AsNumber(FMath::RoundToInt(Proc->GetRequestedPowerPerSecond()))));
    }

    if (NetworkProductionText)
    {
        float Production = 0.f;
        if (UKOEnergySubsystem* Energy = UKOEnergySubsystem::Get(this))
        {
            Production = Energy->GetConsumerNetworkProductionRate(Proc);
        }
        NetworkProductionText->SetText(FText::Format(
            LOCTEXT("NetworkProductionFormat", "{0} /s"),
            FText::AsNumber(FMath::RoundToInt(Production))));
    }
}

void UKOFactoryProcessorWidget::RefreshEventDriven()
{
    UKOFactoryProcessorComponent* Proc = Processor.Get();
    if (!Proc) return;

    if (RecipeText)
    {
        FText RecipeName = LOCTEXT("DefaultRecipeText", "Recipe");
        const FName ActiveId   = Proc->GetActiveRecipeId();
        const FName SelectedId = Proc->GetSelectedRecipe();
        const FName ShownId    = !ActiveId.IsNone() ? ActiveId : SelectedId;
        if (!ShownId.IsNone())
        {
            if (const UKOLoadSubsystem* Load = UKOLoadSubsystem::Get(this))
            {
                if (const FKORecipeRow* Row = Load->FindRecipeRow(ShownId))
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

void UKOFactoryProcessorWidget::HandleProcessorChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload)
{
    const FKOProcessorChangedMessage* Msg = Payload.GetPtr<FKOProcessorChangedMessage>();
    if (!Msg) return;
    if (Msg->Processor.Get() != Processor.Get()) return;

    RefreshEventDriven();
}

#undef LOCTEXT_NAMESPACE
