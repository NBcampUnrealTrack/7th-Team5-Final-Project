// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Skill/KOSkillNodeWidget.h"

void UKOSkillNodeWidget::InitializeNode(FGameplayTag InSkillTag, FSkillCost InCost, ESkillState InState)
{
	SkillTag = InSkillTag;
	SkillCost = InCost;
	CurrentState = InState;

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

	BP_OnSkillStateChanged(CurrentState);
}


