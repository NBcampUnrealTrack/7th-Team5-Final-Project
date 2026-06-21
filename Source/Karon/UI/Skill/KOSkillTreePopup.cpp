// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Skill/KOSkillTreePopup.h"
#include "UI/Skill/KOSkillNodeWidget.h"
#include "UI/Skill/KOSkillTooltipWidget.h"
#include "Component/Skill/KOSkillComponent.h"
#include "Data/Type/KOSkillTypes.h"
#include "Skills/KOSkillLibrary.h"
#include "Subsystem/KOLoadSubsystem.h"

namespace
{
	FText GetExecutionTypeText(ESkillExecutionType Type)
	{
		switch (Type)
		{
		case ESkillExecutionType::Active:          return NSLOCTEXT("KOSkill", "Active", "액티브");
		case ESkillExecutionType::ActiveExtension: return NSLOCTEXT("KOSkill", "ActiveExtension", "행동 추가");
		case ESkillExecutionType::PassiveStat:     return NSLOCTEXT("KOSkill", "Passive", "패시브");
		default:                                   return FText::GetEmpty();
		}
	}
}

UKOSkillTreePopup::UKOSkillTreePopup()
{
	// Back(ESC) 입력 시 자동으로 Deactivate되어 닫힌다. (토글 제거 → Back 일원화)
	InputMode = EKOUIInputMode::Menu;
	bIsBackHandler = true;
}

void UKOSkillTreePopup::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (AController* OwningController = GetOwningPlayer())
	{
		if (UKOSkillComponent* SkillComp = OwningController->FindComponentByClass<UKOSkillComponent>())
		{
			SkillComponent = SkillComp;
		}
	}
	
	SetupAndBindSkillNodes();
}

void UKOSkillTreePopup::NativeDestruct()
{
	SkillComponent = nullptr;
	
	for(UKOSkillNodeWidget* Node : CachedSkillNodes)
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
	
	if (SkillComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill Tree: SkillComponent가 없습니다."));
		return;
	}
	//BP를 통해 가져오므로 슬롯 추가 시 BP에 등록 필요함
	TArray<UKOSkillNodeWidget*> SkillNodes = BP_GetAllSkillNodes();
	
	for (UKOSkillNodeWidget* Node : SkillNodes)
	{
		if (Node == nullptr || Node->SkillName.IsNone()) continue;
		
		const FKOSkillRow* SkillRow = UKOSkillLibrary::GetSkillRow(WorldContext, Node->SkillName);
		
		if (SkillRow)
		{
			ESkillState CurrentState = SkillComponent->GetSkillState(Node->SkillName);
			Node->InitializeNode(Node->SkillName, SkillRow->SkillTag, SkillRow->UnlockCosts, CurrentState);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Skill Tree: SkillName [%s] 에 해당하는 Row를 찾을 수 없습니다."),
				*Node->SkillName.ToString());
		}
	}
}

void UKOSkillTreePopup::SetupAndBindSkillNodes()
{
	for(UKOSkillNodeWidget* Node : CachedSkillNodes)
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
	for(UKOSkillNodeWidget* Node : RetrievedNodes)
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
}

void UKOSkillTreePopup::ShowSkillTooltip(UKOSkillNodeWidget* Node)
{
	if (SkillTooltipWidget == nullptr || Node == nullptr)
	{
		return;
	}

	const FKOSkillRow* SkillRow = UKOSkillLibrary::GetSkillRow(this, Node->SkillName);
	if (SkillRow == nullptr)
	{
		return;
	}

	const UKOLoadSubsystem* LoadSubsystem = UKOLoadSubsystem::Get(this);
	if (LoadSubsystem == nullptr)
	{
		return;
	}

	// SkillName과 동일한 RowName의 실행 데이터 조회
	FText ExecutionTypeText = FText::GetEmpty();
	if (const FKOSkillExecutionRow* ExRow = LoadSubsystem->FindSkillExecutionRow(Node->SkillName))
	{
		ExecutionTypeText = GetExecutionTypeText(ExRow->ExecutionType);
	}

	// 비용 아이템 행 수집
	TArray<FKOItemRow> CostItemRows;
	for (const FSkillCost& Cost : SkillRow->UnlockCosts)
	{
		const FName ItemId = LoadSubsystem->FindItemIdByTag(Cost.ItemTag);
		if (ItemId.IsNone()) continue;

		if (const FKOItemRow* ItemRow = LoadSubsystem->FindItemRow(ItemId))
		{
			CostItemRows.Add(*ItemRow);
		}
	}

	SkillTooltipWidget->InitializeSkillTooltipWidget(*SkillRow, ExecutionTypeText, CostItemRows);
	SkillTooltipWidget->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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
