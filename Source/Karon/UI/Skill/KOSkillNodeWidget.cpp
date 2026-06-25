// Copyright Karon Team 5. All Rights Reserved.

#include "UI/Skill/KOSkillNodeWidget.h"
#include "Subsystem/KOSkillSubsystem.h"
#include "Components/Image.h"

void UKOSkillNodeWidget::InitializeNode(const FName& InSkillName, FGameplayTag InSkillTag,
                                        TArray<FSkillCost> InCost, ESkillState InState)
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

void UKOSkillNodeWidget::NativeOnHovered()
{
	Super::NativeOnHovered();

	if (OnSkillNodeHovered.IsBound())
	{
		OnSkillNodeHovered.Broadcast(this);
	}
}

void UKOSkillNodeWidget::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();

	if (OnSkillNodeUnhovered.IsBound())
	{
		OnSkillNodeUnhovered.Broadcast(this);
	}
}

void UKOSkillNodeWidget::NativeOnClicked()
{
	Super::NativeOnClicked();

	if (!CachedSkillSubsystem.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Skill Node: 저장된 SkillSubsystem이 없습니다."));
		return;
	}

	if (CachedSkillSubsystem.IsValid())
	{
		CachedSkillSubsystem->TryUnlockSkill(SkillName);
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
