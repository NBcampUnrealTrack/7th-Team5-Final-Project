// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Skill/KOSkillNodeWidget.h"
#include "Component/Skill/KOSkillComponent.h"
#include "Components/Image.h"

void UKOSkillNodeWidget::InitializeNode(FName InSkillName, FGameplayTag InSkillTag, TArray<FSkillCost> InCost, ESkillState InState)
{
	SkillName = InSkillName;
	SkillTag = InSkillTag;
	SkillCosts = InCost;
	CurrentState = InState;

	RefreshNode();
}

void UKOSkillNodeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (AController* OwningController = GetOwningPlayer())
	{
		if (UKOSkillComponent* SkillComp = OwningController->FindComponentByClass<UKOSkillComponent>())
		{
			CachedSkillComponent = SkillComp;
		}
	}
	
	RefreshNode();
}

void UKOSkillNodeWidget::NativeDestruct()
{
	CachedSkillComponent = nullptr;

	Super::NativeDestruct();
}

void UKOSkillNodeWidget::NativeOnClicked()
{
	Super::NativeOnClicked();
	
	if (CachedSkillComponent == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill Node: 저장된 SkillComponent가 없습니다."));
		return;
	}

	if (CachedSkillComponent.IsValid())
	{
		CachedSkillComponent->TryUnlockSkill(SkillName);
		RefreshNode();
	}
			
	if (OnSkillNodeClicked.IsBound())
	{
		OnSkillNodeClicked.Broadcast(this);
	}
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
