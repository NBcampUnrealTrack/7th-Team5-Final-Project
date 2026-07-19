// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Skill/KOSkillNodeWidget.h"
#include "Subsystem/KOSkillSubsystem.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Data/KODataTableTypes.h"
#include "UI/Skill/KOSkillDragDropOperation.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputCoreTypes.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UKOSkillNodeWidget::InitializeNode(const FName& InSkillName, FGameplayTag InSkillTag, TArray<FSkillCost> InCost,
                                        ESkillState InState, ESkillExecutionType InExType, UTexture2D* InIcon)
{
	SkillName = InSkillName;
	SkillTag = InSkillTag;
	SkillCosts = InCost;
	CurrentState = InState;
	CurrentExType = InExType;

	if (InIcon && SkillIcon)
	{
		SkillIcon->SetBrushFromTexture(InIcon);
	}

	UE_LOG(LogTemp, Warning, TEXT("Skill Node: %s 스킬 %s 태그로 초기화됨"),
	       *InSkillName.ToString(), *InSkillTag.ToString());

	if (CurrentTypeText)
	{
		SetCurrentTypeText();
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

	if (SkillIcon)
	{
		OriginalBrushTint = SkillIcon->GetBrush().TintColor;
	}
	
	RefreshNode();
}

void UKOSkillNodeWidget::NativeDestruct()
{
	CachedSkillSubsystem = nullptr;

	Super::NativeDestruct();
}

void UKOSkillNodeWidget::NativeOnSelected(bool bBroadcast)
{
	Super::NativeOnSelected(bBroadcast);
	
	if (SelectedImage)
	{
		SelectedImage->SetVisibility(ESlateVisibility::Visible);
	}
}

void UKOSkillNodeWidget::NativeOnDeselected(bool bBroadcast)
{
	Super::NativeOnDeselected(bBroadcast);
	
	if (SelectedImage)
	{
		SelectedImage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UKOSkillNodeWidget::NativeOnClicked()
{
	Super::NativeOnClicked();

	NotifySkillNodeClicked();
}

void UKOSkillNodeWidget::NotifySkillNodeClicked()
{
	if (OnSkillNodeClicked.IsBound())
	{
		OnSkillNodeClicked.Broadcast(this);
	}
}

#define LOCTEXT_NAMESPACE "KOSkillNodeWidget"

void UKOSkillNodeWidget::SetCurrentTypeText()
{
	switch (CurrentExType)
	{
	case ESkillExecutionType::Active:
		CurrentTypeText->SetText(LOCTEXT("SkillType", "액티브"));
		break;

	case ESkillExecutionType::ActiveExtension:
		CurrentTypeText->SetText(LOCTEXT("SkillType", "추가동작"));
		break;

	case ESkillExecutionType::PassiveStat:
		CurrentTypeText->SetText(LOCTEXT("SkillType", "패시브"));
		break;
	}
}
#undef LOCTEXT_NAMESPACE

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

FReply UKOSkillNodeWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry,
                                                          const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && CanDragThisSkill())
	{
		// Preview 단계에서 드래그를 가로채면 NativeOnClicked/SetSelected가 발생하지 않아
		// Tooltip 갱신과 선택 표시(NativeOnSelected/NativeOnDeselected)가 이루어지지 않는다.
		// 드래그 시작 시에도 클릭과 동일하게 이 노드 기준으로 Tooltip 갱신 및 선택 처리를 해준다.
		NotifySkillNodeClicked();
		SetIsSelected(GetSelected() == false, false);

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
	if (BackGroundImage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill Node: BackGroundImage를 찾을 수 없습니다."));
		return;
	}

	switch (CurrentState)
	{
	case ESkillState::Locked:
		{
			FSlateBrush SlateBrush;
			SlateBrush.SetResourceObject(lockedImage);
			BackGroundImage->SetBrush(SlateBrush);
			
			if (SkillIcon)
			{
				SkillIcon->SetBrushTintColor(FLinearColor::White);
			}
			break;
		}

	case ESkillState::CanUnlock:
		{
			FSlateBrush SlateBrush;
			SlateBrush.SetResourceObject(CanUnlockImage);
			BackGroundImage->SetBrush(SlateBrush);
			
			if (SkillIcon)
			{
				SkillIcon->SetBrushTintColor(OriginalBrushTint);
			}
			break;
		}

	case ESkillState::Unlocked:
		{
			FSlateBrush SlateBrush;
			SlateBrush.SetResourceObject(UnlockedImage);
			BackGroundImage->SetBrush(SlateBrush);
			
			if (SkillIcon)
			{
				SkillIcon->SetBrushTintColor(OriginalBrushTint);
			}
			break;
		}
	}
}
