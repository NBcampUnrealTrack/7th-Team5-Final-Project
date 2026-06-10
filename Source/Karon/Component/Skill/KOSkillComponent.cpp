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

bool UKOSkillComponent::TryUnlockSkill(FName SkillName)
{
	if (SkillName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: 스킬명이 없습니다."));
		return false;
	}
		
	const UKOLoadSubsystem* LS = UKOLoadSubsystem::Get(GetOwner());
	if (!LS)
	{
		return false;
	}
	const FKOSkillRow* Row = LS->FindSkillRow(SkillName);
	if (Row != nullptr)
	{	/** 선행 스킬이 해금되었는지 체크 */
		if (ArePrerequisitesMet(*Row) == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillComponent: 선행 조건을 미충족 했습니다."));
			return false;
		}
	}

	ESkillState* State = SkillStates.Find(SkillName);
	if (State == nullptr || *State != ESkillState::CanUnlock)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: SkillState가 없거나 CanUnlock이 아닙니다."));
		return false;
	}

	 *State = ESkillState::Unlocked;
	ReevaluateAllSkillStates();
	OnSkillStateChanged.Broadcast();
	return true;
}

ESkillState UKOSkillComponent::GetSkillState(FName SkillName) const
{
	const ESkillState* State = GetSkillInfo(SkillName);
	return State ? *State : ESkillState::Locked;
}

bool UKOSkillComponent::IsUnlocked(FName SkillName) const
{
	return GetSkillState(SkillName) == ESkillState::Unlocked;
}

FGameplayTagContainer UKOSkillComponent::GetUnlockedSkillTags() const
{
	const UKOLoadSubsystem* LS = UKOLoadSubsystem::Get(GetOwner());
	FGameplayTagContainer Tags;
	if (LS == nullptr)
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

void UKOSkillComponent::GetAllSkillNames(TArray<FName>& Out) const
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

	for (const FName& SkillName : AllSkillIds)
	{
		SkillStates.Add(SkillName, ESkillState::Locked);
	}

	ReevaluateAllSkillStates();
}

void UKOSkillComponent::ReevaluateAllSkillStates()
{
	const UKOLoadSubsystem* LS = UKOLoadSubsystem::Get(GetOwner());
	if (LS == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReevaluateAllSkillStates: LS가 없습니다."))
		return;
	}

	for (TPair<FName, ESkillState>& Pair : SkillStates)
	{
		if (Pair.Value == ESkillState::Unlocked)
		{
			continue; // 이미 해금된 스킬은 건드리지 않음
		}

		const FKOSkillRow* Row = LS->FindSkillRow(Pair.Key);
		if (Row == nullptr)
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

const ESkillState* UKOSkillComponent::GetSkillInfo(FName SkillName) const
{
	return SkillStates.Find(SkillName);
}
