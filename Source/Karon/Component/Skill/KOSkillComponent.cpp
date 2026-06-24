// Copyright Karon Team 5. All Rights Reserved.

#include "KOSkillComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Data/Type/KOSkillTypes.h"
#include "Game/KOPlayerController.h"
#include "Game/KOPlayerState.h"
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

		AKOPlayerState* PS = PC->GetPlayerState<AKOPlayerState>();
		if (PS)
		{
			CachedASC = PS->GetAbilitySystemComponent();
		}
	}

	InitializeSkillStates();
}

void UKOSkillComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	CachedLoadSubsystem = nullptr;
	CachedInventoryComponent = nullptr;

	Super::EndPlay(EndPlayReason);
}

bool UKOSkillComponent::TryUnlockSkill(const FName& SkillName)
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

	if (CachedASC == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: 캐싱된 AbilitySystemComponent가 없습니다."));
		return false;
	}

	const FKOSkillRow* Row = CachedLoadSubsystem->FindSkillRow(SkillName);
	const FKOSkillExecutionRow* ExRow = CachedLoadSubsystem->FindSkillExecutionRow(SkillName);

	if (Row == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: [%s] 스킬 데이터를 찾을 수 없습니다."), *SkillName.ToString());
		return false;
	}

	if (ExRow == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillComponent: [%s] 스킬 실행 데이터를 찾을 수 없습니다."), *SkillName.ToString());
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
				UE_LOG(LogTemp, Warning, TEXT("SkillComponent: 비용 태그 [%s]에 해당하는 아이템이 없습니다."),
				       *Cost.ItemTag.ToString());
				return false;
			}
			if (!CachedInventoryComponent->HasEnoughItems(ItemId, Cost.Amount))
			{
				UE_LOG(LogTemp, Warning, TEXT("SkillComponent: [%s] 수량 부족 (필요: %d)"),
				       *ItemId.ToString(), Cost.Amount);
				return false;
			}
		}

		for (const FSkillCost& Cost : Row->UnlockCosts)
		{
			CachedInventoryComponent->TryRemoveItem(CachedLoadSubsystem->FindItemIdByTag(Cost.ItemTag), Cost.Amount);
		}
	}

	FGameplayEffectContextHandle EffectContext = CachedASC->MakeEffectContext();

	if (ExRow->ExecutionType == ESkillExecutionType::Active && ExRow->AbilityClass)
	{
		FGameplayAbilitySpec* ExistingSpec = CachedASC->FindAbilitySpecFromClass(ExRow->AbilityClass);
		if (ExistingSpec)
		{
			UE_LOG(LogTemp, Warning, TEXT("이미 태그가 부여되어 있습니다. 어캐들어옴?"));
			return false;
		}
		
		FGameplayAbilitySpec NewSpec(ExRow->AbilityClass, 1, -1);
		
		FGameplayAbilitySpecHandle SpecHandle = CachedASC->GiveAbility(NewSpec);
		if (SpecHandle.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("%s 스킬 해금 후 태그 지급 완료"), *SkillName.ToString());
			bool bSuccess = CachedASC->TryActivateAbility(SpecHandle);
			UE_LOG(LogTemp, Warning, TEXT("Handle- 스킬 강제 발동 테스트 결과: %s"), bSuccess ? TEXT("성공"): TEXT("실패"));
		}
		
		if (ExRow->AbilityClass)
		{
			bool bSuccess = CachedASC->TryActivateAbilityByClass(ExRow->AbilityClass);
			UE_LOG(LogTemp, Warning, TEXT("ExRow - 스킬 강제 발동 테스트 결과: %s"), bSuccess ? TEXT("성공"): TEXT("실패"));
		}
	}
	else if (ExRow->ExecutionType == ESkillExecutionType::PassiveStat && ExRow->PassiveEffectClass)
	{
		FGameplayEffectSpecHandle SpecHandle = CachedASC->MakeOutgoingSpec(
			ExRow->PassiveEffectClass, 1.f, EffectContext);
		if (!SpecHandle.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("TryUnlockSkill: [%s] Passive SpecHandle 생성 실패"), *SkillName.ToString());
			return false;
		}
		CachedASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

		for (FGameplayAbilitySpec& AbilitySpec : CachedASC->GetActivatableAbilities())
		{
			CachedASC->MarkAbilitySpecDirty(AbilitySpec);
		}
	}

	*State = ESkillState::Unlocked;
	ReevaluateAllSkillStates();
	return true;
}

ESkillState UKOSkillComponent::GetSkillState(const FName& SkillName) const
{
	const ESkillState* State = GetSkillInfo(SkillName);
	return State ? *State : ESkillState::Locked;
}

bool UKOSkillComponent::IsUnlocked(const FName& SkillName) const
{
	return GetSkillState(SkillName) == ESkillState::Unlocked;
}

bool UKOSkillComponent::HasActiveAbilityInASC(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	if (CachedASC)
	{
		const TArray<FGameplayAbilitySpec>& AbilitySpecs = CachedASC->GetActivatableAbilities();
		for (const FGameplayAbilitySpec& Spec : AbilitySpecs)
		{
			if (Spec.Ability && Spec.Ability->GetClass() == AbilityClass)
			{
				return true;
			}
		}
	}
	return false;
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
