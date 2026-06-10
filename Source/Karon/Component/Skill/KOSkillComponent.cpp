// Copyright Karon Team 5. All Rights Reserved.

#include "KOSkillComponent.h"
#include "Data/Type/KOSkillTypes.h"
#include "Game/KOPlayerController.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Subsystem/KOLoadSubsystem.h"

UKOSkillComponent::UKOSkillComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UKOSkillComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedLoadSubsystem = UKOLoadSubsystem::Get(GetOwner());

	if (AKOPlayerController* PC = Cast<AKOPlayerController>(GetOwner()))
	{
		CachedInventoryComponent = PC->FindComponentByClass<UKOInventoryComponent>();
	}

	InitializeSkillStates();
}

void UKOSkillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CachedLoadSubsystem = nullptr;
	CachedInventoryComponent = nullptr;

	Super::EndPlay(EndPlayReason);
}

bool UKOSkillComponent::TryUnlockSkill(FName SkillName)
{
	if (SkillName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: 스킬명이 없습니다."));
		return false;
	}
	if (IsUnlocked(SkillName))
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: 스킬이 이미 해금된 상태입니다."));
		return false;
	}

	if (CachedLoadSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: 캐싱된 LoadSubsystem이 없습니다."));
		return false;
	}
	const FKOSkillRow* Row = CachedLoadSubsystem->FindSkillRow(SkillName);

	if (Row == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: [%s] 스킬 데이터를 찾을 수 없습니다."), *SkillName.ToString());
		return false;
	}

	if (ArePrerequisitesMet(*Row) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: 선행 조건을 미충족 했습니다."));
		return false;
	}

	ESkillState* State = SkillStates.Find(SkillName);
	if (State == nullptr || *State != ESkillState::CanUnlock)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: SkillState가 없거나 CanUnlock이 아닙니다."));
		return false;
	}

	if (!Row->UnlockCosts.IsEmpty())
	{
		if (CachedInventoryComponent == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillComponent: 비용이 있는 스킬인데 InventoryComponent가 없습니다."));
			return false;
		}

		for (const FSkillCost& Cost : Row->UnlockCosts)
		{
			const FName ItemId = CachedLoadSubsystem->FindItemIdByTag(Cost.ItemTag);
			if (ItemId.IsNone())
			{
				UE_LOG(LogTemp, Warning, TEXT("SkillComponent: 비용 태그 [%s]에 해당하는 아이템이 없습니다."), *Cost.ItemTag.ToString());
				return false;
			}
			if (!CachedInventoryComponent->HasEnoughItems(ItemId, Cost.Amount))
			{
				UE_LOG(LogTemp, Warning, TEXT("SkillComponent: [%s] 수량 부족 (필요: %d)"), *ItemId.ToString(), Cost.Amount);
				return false;
			}
		}

		for (const FSkillCost& Cost : Row->UnlockCosts)
		{
			CachedInventoryComponent->TryRemoveItem(CachedLoadSubsystem->FindItemIdByTag(Cost.ItemTag), Cost.Amount);
		}
	}

	*State = ESkillState::Unlocked;
	ReevaluateAllSkillStates();
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
	FGameplayTagContainer Tags;
	if (CachedLoadSubsystem == nullptr)
	{
		return Tags;
	}

	for (const TPair<FName, ESkillState>& Pair : SkillStates)
	{
		if (Pair.Value != ESkillState::Unlocked)
		{
			continue;
		}

		const FKOSkillRow* Row = CachedLoadSubsystem->FindSkillRow(Pair.Key);
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

	if (CachedLoadSubsystem == nullptr)
	{
		return;
	}

	TArray<FName> AllSkillIds;
	CachedLoadSubsystem->GetAllSkillIds(AllSkillIds);

	for (const FName& SkillName : AllSkillIds)
	{
		SkillStates.Add(SkillName, ESkillState::Locked);
	}

	ReevaluateAllSkillStates();
}

void UKOSkillComponent::ReevaluateAllSkillStates()
{
	if (CachedLoadSubsystem == nullptr)
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

		const FKOSkillRow* Row = CachedLoadSubsystem->FindSkillRow(Pair.Key);
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
