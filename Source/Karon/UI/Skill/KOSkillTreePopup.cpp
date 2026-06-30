// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Skill/KOSkillTreePopup.h"
#include "UI/Skill/KOSkillNodeWidget.h"
#include "UI/Skill/KOSkillTooltipWidget.h"
#include "Subsystem/KOSkillSubsystem.h"
#include "Data/Type/KOSkillTypes.h"
#include "Skills/KOSkillLibrary.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ScrollBox.h"

namespace
{
	FText GetExecutionTypeText(ESkillExecutionType Type)
	{
		switch (Type)
		{
		case ESkillExecutionType::Active: return NSLOCTEXT("KOSkill", "Active", "액티브");
		case ESkillExecutionType::ActiveExtension: return NSLOCTEXT("KOSkill", "ActiveExtension", "행동 추가");
		case ESkillExecutionType::PassiveStat: return NSLOCTEXT("KOSkill", "Passive", "패시브");
		default: return FText::GetEmpty();
		}
	}
}

void UKOSkillTreePopup::NativeConstruct()
{
	Super::NativeConstruct();

	if (ULocalPlayer* LP = GetOwningLocalPlayer())
	{
		SkillSubsystem = LP->GetSubsystem<UKOSkillSubsystem>();
	}

	CachedLoadSubsystem = UKOLoadSubsystem::Get(this);

	SetupAndBindSkillNodes();
	ScrollBox->ScrollToEnd();
}

void UKOSkillTreePopup::NativeDestruct()
{
	SkillSubsystem = nullptr;

	for (UKOSkillNodeWidget* Node : CachedSkillNodes)
	{
		if (IsValid(Node))
		{
			Node->OnSkillNodeClicked.RemoveAll(this);
			Node->OnSkillNodeHovered.RemoveAll(this);
			Node->OnSkillNodeUnhovered.RemoveAll(this);
		}
	}
	CachedSkillNodes.Empty();

	HideSkillTooltip();

	Super::NativeDestruct();
}

void UKOSkillTreePopup::RefreshAllSkillNodes() const
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	const UObject* WorldContext = GetWorld();
	if (OwningPawn == nullptr || WorldContext == nullptr)
	{
		return;
	}

	if (SkillSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill Tree: SkillSubsystem이 없습니다."));
		return;
	}
	/**   BP를 통해 가져오므로 슬롯 추가 시 BP에 등록 필요함   */
	TArray<UKOSkillNodeWidget*> SkillNodes = BP_GetAllSkillNodes();

	for (UKOSkillNodeWidget* Node : SkillNodes)
	{
		if (Node == nullptr || Node->GetSkillName().IsNone()) continue;

		const FKOSkillRow* SkillRow = UKOSkillLibrary::GetSkillRow(WorldContext, Node->GetSkillName());

		if (SkillRow)
		{
			if (SkillRow->Icon.Get() == nullptr && SkillRow->Icon.IsNull() == false)
			{
				UE_LOG(LogTemp, Display, TEXT("SkillTree: Icon을 로딩합니다"));
				(void)CachedLoadSubsystem->ResolveSkillIcon(Node->GetSkillName());
			}
			ESkillState CurrentState = SkillSubsystem->GetSkillState(Node->GetSkillName());
			Node->InitializeNode(Node->GetSkillName(), SkillRow->SkillTag, SkillRow->UnlockCosts, CurrentState);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Skill Tree: SkillName [%s] 에 해당하는 Row를 찾을 수 없습니다."),
			       *Node->GetSkillName().ToString());
		}
	}
}

void UKOSkillTreePopup::RefreshActiveTooltip(UKOSkillNodeWidget* Node)
{
	if (SkillTooltipWidget == nullptr || Node == nullptr)
	{
		return;
	}

	const FKOSkillRow* SkillRow = UKOSkillLibrary::GetSkillRow(this, Node->GetSkillName());
	if (SkillRow == nullptr || CachedLoadSubsystem == nullptr)
	{
		return;
	}

	TArray<FKOItemRow> CostItemRows;
	for (const FSkillCost& Cost : SkillRow->UnlockCosts)
	{
		const FName ItemId = CachedLoadSubsystem->FindItemIdByTag(Cost.ItemTag);
		if (ItemId.IsNone()) continue;

		if (const FKOItemRow* ItemRow = CachedLoadSubsystem->FindItemRow(ItemId))
		{
			CostItemRows.Add(*ItemRow);
		}
	}

	SkillTooltipWidget->RefreshCostWidget(Node->GetCurrentState(), CostItemRows);
}

void UKOSkillTreePopup::SetupAndBindSkillNodes()
{
	for (UKOSkillNodeWidget* Node : CachedSkillNodes)
	{
		if (IsValid(Node))
		{
			Node->OnSkillNodeClicked.RemoveAll(this);
			Node->OnSkillNodeHovered.RemoveAll(this);
			Node->OnSkillNodeUnhovered.RemoveAll(this);
		}
	}
	CachedSkillNodes.Empty();

	TArray<UKOSkillNodeWidget*> RetrievedNodes = BP_GetAllSkillNodes();
	for (UKOSkillNodeWidget* Node : RetrievedNodes)
	{
		if (IsValid(Node))
		{
			CachedSkillNodes.Add(Node);

			Node->OnSkillNodeClicked.AddUObject(this, &UKOSkillTreePopup::HandleSkillNodeClicked);
			Node->OnSkillNodeHovered.AddUObject(this, &UKOSkillTreePopup::ShowSkillTooltip);
			Node->OnSkillNodeUnhovered.AddUObject(this, &UKOSkillTreePopup::HandleSkillNodeUnhovered);
		}
	}
	RefreshAllSkillNodes();
}

void UKOSkillTreePopup::HandleSkillNodeClicked(UKOSkillNodeWidget* ClickedNode)
{
	if (ClickedNode == nullptr)
	{
		return;
	}
	RefreshAllSkillNodes();
	RefreshActiveTooltip(ClickedNode);
}

void UKOSkillTreePopup::ShowSkillTooltip(UKOSkillNodeWidget* Node)
{
	if (SkillTooltipWidget == nullptr || Node == nullptr)
	{
		return;
	}

	const FKOSkillRow* SkillRow = UKOSkillLibrary::GetSkillRow(this, Node->GetSkillName());
	if (SkillRow == nullptr)
	{
		return;
	}

	if (CachedLoadSubsystem == nullptr)
	{
		return;
	}

	// SkillName과 동일한 RowName의 실행 데이터 조회
	FText ExecutionTypeText = FText::GetEmpty();
	if (const FKOSkillExecutionRow* ExRow = CachedLoadSubsystem->FindSkillExecutionRow(Node->GetSkillName()))
	{
		ExecutionTypeText = GetExecutionTypeText(ExRow->ExecutionType);
	}

	// 비용 아이템 행 수집
	TArray<FKOItemRow> CostItemRows;
	for (const FSkillCost& Cost : SkillRow->UnlockCosts)
	{
		const FName ItemId = CachedLoadSubsystem->FindItemIdByTag(Cost.ItemTag);
		if (ItemId.IsNone()) continue;

		if (const FKOItemRow* ItemRow = CachedLoadSubsystem->FindItemRow(ItemId))
		{
			CostItemRows.Add(*ItemRow);
		}
	}

	ESkillState SkillState = Node->GetCurrentState();

	SkillTooltipWidget->InitializeSkillTooltipWidget(*SkillRow, SkillState, ExecutionTypeText, CostItemRows);
	SkillTooltipWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	// 노드 버튼 오른쪽에 툴팁 배치
	if (UCanvasPanelSlot* TooltipSlot = Cast<UCanvasPanelSlot>(SkillTooltipWidget->Slot))
	{
		if (UCanvasPanelSlot* NodeSlot = Cast<UCanvasPanelSlot>(Node->Slot))
		{
			TooltipSlot->SetAlignment(FVector2D(0.f, 0.5f));

			const FVector2D NodePos = NodeSlot->GetPosition();
			const FVector2D NodeSize = NodeSlot->GetSize();
			const FVector2D NodeAlignment = NodeSlot->GetAlignment();

			FVector2D NodeRelativeTopLeft = NodePos - (NodeSize * NodeAlignment);

			FVector2D NewPosition;
			NewPosition.X = NodeRelativeTopLeft.X + NodeSize.X + TooltipInterval;
			NewPosition.Y = NodeRelativeTopLeft.Y + (NodeSize.Y * 0.5f);

			TooltipSlot->SetPosition(NewPosition);
		}
	}
}

void UKOSkillTreePopup::HandleSkillNodeUnhovered(UKOSkillNodeWidget* /*Node*/)
{
	HideSkillTooltip();
}

void UKOSkillTreePopup::HideSkillTooltip()
{
	if (SkillTooltipWidget)
	{
		SkillTooltipWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}
