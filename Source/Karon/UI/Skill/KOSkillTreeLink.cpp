// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Skill/KOSkillTreeLink.h"
#include "UI/Skill/KOSkillNodeWidget.h"

#include "Components/Image.h"

void UKOSkillTreeLink::NativeConstruct()
{
	Super::NativeConstruct();

	RefreshLink();
}

void UKOSkillTreeLink::RefreshLink()
{
	if (LinkImage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill Tree Link: LinkImage를 찾을 수 없습니다."));
		return;
	}

	if (TargetNode == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill Tree Link: TargetNode가 지정되지 않았습니다."));
		return;
	}

	switch (TargetNode->GetCurrentState())
	{
	case ESkillState::Locked:
		{
			FSlateBrush SlateBrush;
			SlateBrush.SetResourceObject(LockedImage);
			LinkImage->SetBrush(SlateBrush);
			break;
		}

	case ESkillState::CanUnlock:
		{
			FSlateBrush SlateBrush;
			SlateBrush.SetResourceObject(CanUnlockImage);
			LinkImage->SetBrush(SlateBrush);
			break;
		}

	case ESkillState::Unlocked:
		{
			FSlateBrush SlateBrush;
			SlateBrush.SetResourceObject(UnlockedImage);
			LinkImage->SetBrush(SlateBrush);
			break;
		}
	}
}