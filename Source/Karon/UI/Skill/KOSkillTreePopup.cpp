// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Skill/KOSkillTreePopup.h"
#include "UI/Skill/KOSkillNodeWidget.h"
#include "UI/Skill/KOSkillTreeLink.h"
#include "UI/Skill/KOSkillTooltipWidget.h"
#include "UI/KOToastMessageWidget.h"
#include "Subsystem/KOSkillSubsystem.h"
#include "Data/Type/KOSkillTypes.h"
#include "Skills/KOSkillLibrary.h"
#include "Subsystem/KOLoadSubsystem.h"

#include "Components/ScrollBox.h"
#include "Groups/CommonButtonGroupBase.h"

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

	/**   BP를 통해 가져오므로 연결선 추가 시 BP에 등록 필요함   */
	CachedSkillLinks = BP_GetAllSkillLinks();
	RefreshAllSkillLinks();

	ScrollBox->ScrollToEnd();

	if (SkillTooltipWidget)
	{
		SkillTooltipWidget->OnConfirmed.AddDynamic(this, &UKOSkillTreePopup::HandleTooltipConfirmed);
	}
	
	if (CachedSkillNodes.IsValidIndex(InitialSkillNodeIndex))
	{
		ShowSkillTooltip(CachedSkillNodes[InitialSkillNodeIndex]);
	}
	
	SkillNodeGroup = NewObject<UCommonButtonGroupBase>(this);
	
	if (SkillNodeGroup && CachedSkillNodes.IsEmpty() == false)
	{
		for (UKOSkillNodeWidget* Node : CachedSkillNodes)
		{
			SkillNodeGroup->AddWidget(Node);
		}
		
		SkillNodeGroup->DeselectAll();
	}
}

void UKOSkillTreePopup::NativeDestruct()
{		
	SkillSubsystem = nullptr;

	if (SkillTooltipWidget)
	{
		SkillTooltipWidget->OnConfirmed.RemoveAll(this);
	}

	for (UKOSkillNodeWidget* Node : CachedSkillNodes)
	{
		if (IsValid(Node))
		{
			Node->OnSkillNodeClicked.RemoveAll(this);
		}
	}
	CachedSkillNodes.Empty();
	CachedSkillLinks.Empty();
	ActiveTooltipNode = nullptr;

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
		
		const FKOSkillExecutionRow* SkillExRow = CachedLoadSubsystem->FindSkillExecutionRow(Node->GetSkillName());

		if (SkillRow && SkillExRow)
		{
			if (SkillRow->Icon.Get() == nullptr && SkillRow->Icon.IsNull() == false)
			{
				UE_LOG(LogTemp, Display, TEXT("SkillTree: Icon을 로딩합니다"));
				(void)CachedLoadSubsystem->ResolveSkillIcon(Node->GetSkillName());
			}
			
			ESkillState CurrentState = SkillSubsystem->GetSkillState(Node->GetSkillName());
			ESkillExecutionType ExType = SkillExRow->ExecutionType;
			
			Node->InitializeNode(Node->GetSkillName(), SkillRow->SkillTag, SkillRow->UnlockCosts,
			                     CurrentState, ExType, SkillRow->Icon.Get());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Skill Tree: SkillName [%s] 에 해당하는 Row 또는 ExRow를 찾을 수 없습니다."),
			       *Node->GetSkillName().ToString());
		}
	}
}

void UKOSkillTreePopup::RefreshAllSkillLinks() const
{
	for (UKOSkillTreeLink* Link : CachedSkillLinks)
	{
		if (IsValid(Link))
		{
			Link->RefreshLink();
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

	ShowSkillTooltip(ClickedNode);
}

void UKOSkillTreePopup::HandleTooltipConfirmed()
{
	if (IsValid(ActiveTooltipNode) == false)
	{
		return;
	}

	if (SkillSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill Tree: SkillSubsystem이 없습니다."));
		return;
	}

	const FName SkillName = ActiveTooltipNode->GetSkillName();
	if (SkillName.IsNone())
	{
		return;
	}

	const bool bUnlocked = SkillSubsystem->TryUnlockSkill(SkillName);

	if (bUnlocked == false)
	{
		const FText& ReasonText = SkillSubsystem->GetLastUnlockFailureReason();

		if (!ReasonText.IsEmpty() && ToastMessageWidget)
		{
			ToastMessageWidget->ShowMessage(ReasonText, 2.0f);
		}

		return;
	}

	RefreshAllSkillNodes();
	RefreshAllSkillLinks();
	RefreshActiveTooltip(ActiveTooltipNode);
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

	ActiveTooltipNode = Node;

	SkillTooltipWidget->InitializeSkillTooltipWidget(*SkillRow, SkillState, ExecutionTypeText, CostItemRows);
}
