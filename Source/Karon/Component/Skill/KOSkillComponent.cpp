// Copyright Karon Team 5. All Rights Reserved.

#include "KOSkillComponent.h"
#include "Data/Type/KOSkillTypes.h"
#include "Subsystem/KOLoadSubsystem.h"

UKOSkillComponent::UKOSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UKOSkillComponent::BeginPlay()
{
	Super::BeginPlay();
	InitializeSkillStates();
}

bool UKOSkillComponent::TryUnlockSkill(FName SkillId)
{
	if (SkillId.IsNone())
	{
		return false;
	}

	ESkillState* State = SkillStates.Find(SkillId);
	if (!State || *State != ESkillState::CanUnlock)
	{
		return false;
	}

	*State = ESkillState::Unlocked;
	ReevaluateAllSkillStates();
	return true;
}

ESkillState UKOSkillComponent::GetSkillState(FName SkillId) const
{
	const ESkillState* State = SkillStates.Find(SkillId);
	return State ? *State : ESkillState::Locked;
}

bool UKOSkillComponent::IsUnlocked(FName SkillId) const
{
	return GetSkillState(SkillId) == ESkillState::Unlocked;
}

FGameplayTagContainer UKOSkillComponent::GetUnlockedSkillTags() const
{
	const UKOLoadSubsystem* LS = UKOLoadSubsystem::Get(GetOwner());
	FGameplayTagContainer Tags;
	if (!LS)
	{
		return Tags;
	}

	for (const TPair<FName, ESkillState>& Pair : SkillStates)
	{
		if (Pair.Value != ESkillState::Unlocked)
		{
			continue;
		}

		const FKOSkillRow* Row = LS->FindSkillRow(Pair.Key);
		if (Row && Row->SkillTag.IsValid())
		{
			Tags.AddTag(Row->SkillTag);
		}
	}
	return Tags;
}

void UKOSkillComponent::GetAllSkillIds(TArray<FName>& Out) const
{
	SkillStates.GetKeys(Out);
}

void UKOSkillComponent::InitializeSkillStates()
{
	SkillStates.Empty();

	const UKOLoadSubsystem* LS = UKOLoadSubsystem::Get(GetOwner());
	if (!LS)
	{
		return;
	}

	TArray<FName> AllSkillIds;
	LS->GetAllSkillIds(AllSkillIds);

	for (const FName& SkillId : AllSkillIds)
	{
		SkillStates.Add(SkillId, ESkillState::Locked);
	}

	ReevaluateAllSkillStates();
}

void UKOSkillComponent::ReevaluateAllSkillStates()
{
	const UKOLoadSubsystem* LS = UKOLoadSubsystem::Get(GetOwner());
	if (!LS)
	{
		return;
	}

	for (TPair<FName, ESkillState>& Pair : SkillStates)
	{
		if (Pair.Value == ESkillState::Unlocked)
		{
			continue; // 이미 해금된 스킬은 건드리지 않음
		}

		const FKOSkillRow* Row = LS->FindSkillRow(Pair.Key);
		if (!Row)
		{
			continue;
		}

		Pair.Value = ArePrerequisitesMet(*Row) ? ESkillState::CanUnlock : ESkillState::Locked;
	}
}

bool UKOSkillComponent::ArePrerequisitesMet(const FKOSkillRow& Row) const
{
	if (Row.PrerequisiteSkillTags.IsEmpty())
	{
		return true;
	}

	return GetUnlockedSkillTags().HasAll(Row.PrerequisiteSkillTags);
}

const ESkillState* UKOSkillComponent::GetSkillInfo(FName SkillId) const
{
	return SkillStates.Find(SkillId);
}
