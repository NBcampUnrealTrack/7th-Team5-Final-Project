// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Skill/KOSkillNodeWidget.h"

void UKOSkillNodeWidget::InitializeNode(FGameplayTag InSkillTag, FSkillCost InCost, ESkillState InState)
{
	SkillTag = InSkillTag;
	SkillCost = InCost;
	CurrentState = InState;
	//테스트용 임시 강제 해제가능 지정
	CurrentState = ESkillState::CanUnlock;
	BP_OnSkillStateChanged(CurrentState);
}

void UKOSkillNodeWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UKOSkillNodeWidget::NativeOnClicked()
{
	Super::NativeOnClicked();
	
	if (CurrentState != ESkillState::CanUnlock)
	{
		return;
	}
	
	APlayerController* PC = GetOwningPlayer();
	if (PC == nullptr)
	{
		return;
	}
	// TODO_CSH MVP이후 인벤토리에서 검증주고받기 필요
	// 테스트용 강제 해금 변경
	CurrentState = ESkillState::Unlocked;
	BP_OnSkillStateChanged(CurrentState);
}


