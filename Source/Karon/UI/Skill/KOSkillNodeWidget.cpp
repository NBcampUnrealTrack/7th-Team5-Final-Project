// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Skill/KOSkillNodeWidget.h"
#include "Subsystem/KOSkillSubsystem.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Data/KODataTableTypes.h"
#include "UI/Skill/KOSkillDragDropOperation.h"
#include "Components/Image.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputCoreTypes.h"

void UKOSkillNodeWidget::InitializeNode(const FName& InSkillName, FGameplayTag InSkillTag,
                                        TArray<FSkillCost> InCost, ESkillState InState, UTexture2D* InIcon)
{
	SkillName = InSkillName;
	SkillTag = InSkillTag;
	SkillCosts = InCost;
	CurrentState = InState;
	
	if (InIcon && SkillIcon)
	{
		SkillIcon->SetBrushFromTexture(InIcon);
	}

	RefreshNode();
}

void UKOSkillNodeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		CachedSkillSubsystem = LP->GetSubsystem<UKOSkillSubsystem>();
	}

	RefreshNode();
}

void UKOSkillNodeWidget::NativeDestruct()
{
	CachedSkillSubsystem = nullptr;

	Super::NativeDestruct();
}

void UKOSkillNodeWidget::NativeOnClicked()
{
	Super::NativeOnClicked();

	if (OnSkillNodeClicked.IsBound())
	{
		OnSkillNodeClicked.Broadcast(this);
	}
}

void UKOSkillNodeWidget::ExecuteUnlock()
{
	if (CachedSkillSubsystem.IsValid())
	{
		CachedSkillSubsystem->TryUnlockSkill(SkillName);
		RefreshNode();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill Node: 저장된 SkillSubsystem이 없습니다."));
	}
}

FReply UKOSkillNodeWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && CanDragThisSkill())
	{
		FEventReply Reply = UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this,
		                                                                 EKeys::LeftMouseButton);
		return Reply.NativeReply;
	}

	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

void UKOSkillNodeWidget::NativeOnDragDetected(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!CanDragThisSkill())
	{
		return;
	}

	UTexture2D* Icon = nullptr;
	if (UKOLoadSubsystem* LS = UKOLoadSubsystem::Get(this))
	{
		Icon = LS->ResolveSkillIcon(SkillName);
	}

	UKOSkillDragDropOperation* DragOp = UKOSkillDragDropOperation::Create(
		this,
		SkillName,
		SkillTag,
		Icon,
		DragVisualSize,
		DragVisualOpacity
	);

	OutOperation = DragOp;
}

bool UKOSkillNodeWidget::CanDragThisSkill() const
{
	if (SkillName.IsNone())
	{
		return false;
	}

	if (CurrentState == ESkillState::Unlocked)
	{
		if (UKOLoadSubsystem* LS = UKOLoadSubsystem::Get(this))
		{
			if (const FKOSkillExecutionRow* ExRow = LS->FindSkillExecutionRow(SkillName))
			{
				return ExRow->ExecutionType == ESkillExecutionType::Active;
			}
		}
	}

	return false;
}

void UKOSkillNodeWidget::RefreshNode()
{
	if (OverlayImage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill Node: OverlayImage를 찾을 수 없습니다."));
		return;
	}

	switch (CurrentState)
	{
	case ESkillState::Locked:
		OverlayImage->SetColorAndOpacity(LockedColor);
		break;

	case ESkillState::CanUnlock:
		OverlayImage->SetColorAndOpacity(CanUnlockColor);
		break;

	case ESkillState::Unlocked:
		OverlayImage->SetColorAndOpacity(UnlockedColor);
		break;
	}
}
