// Copyright Karon Team 5. All Rights Reserved.
#include "UI/Interaction/KOFactoryProcessorWidget.h"

#include "Building/KOBaseBuilding.h"
#include "Component/KOFactoryProcessorComponent.h"
#include "Component/KOInteractionComponent.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Data/KODataTableTypes.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Items/KOItemLibrary.h"
#include "Items/KOItemSlot.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "TimerManager.h"

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

    Processor.Reset();
    TargetBuilding.Reset();

    Super::NativeOnDeactivated();
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
}

#undef LOCTEXT_NAMESPACE
