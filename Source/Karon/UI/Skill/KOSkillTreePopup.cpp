// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Skill/KOSkillTreePopup.h"
#include "UI/Skill/KOSkillNodeWidget.h"

UKOSkillTreePopup::UKOSkillTreePopup()
{
	// Back(ESC) 입력 시 자동으로 Deactivate되어 닫힌다. (토글 제거 → Back 일원화)
	InputMode = EKOUIInputMode::Menu;
	bIsBackHandler = true;
}

void UKOSkillTreePopup::NativeConstruct()
{
	Super::NativeConstruct();
	
	RefreshAllSkillNodes();
}

void UKOSkillTreePopup::NativeOnDeactivated()
{
	
	
	Super::NativeOnDeactivated();
}

void UKOSkillTreePopup::RefreshAllSkillNodes() const
{
	APawn* OwningPawn = GetOwningPlayerPawn();
	if (OwningPawn == nullptr)
	{
		return;
	}
	
	TArray<UKOSkillNodeWidget*> SkillNodes = BP_GetAllSkillNodes();
	
	for (UKOSkillNodeWidget* Node : SkillNodes)
	{
		if (Node == nullptr) return;
		//임시 강제 해금 가능 등록
		ESkillState TargetState = ESkillState::CanUnlock;
		/* 저장위치에서 값을 받아와 노드 활성화(State가 아닌 SkillComponent가 될수도 있음)
		if (PlayerState->IsSkillUnlocked(Node->SkillTag))
		{
			TargetState = ESkillState::Unlocked;
		}
		else if (!PlayerState->IsSkillUnlocked(Node->SkillTag))
		{
			TargetState = ESkillState::Locked;
		}
		*/
		Node->InitializeNode(Node->SkillTag, Node->SkillCost, TargetState);
	}
}
