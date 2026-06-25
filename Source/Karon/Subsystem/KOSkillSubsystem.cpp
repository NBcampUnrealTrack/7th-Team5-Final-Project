// Copyright Karon Team 5. All Rights Reserved.

#include "KOSkillSubsystem.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Data/Type/KOSkillTypes.h"
#include "Game/KOPlayerController.h"
#include "Game/KOPlayerState.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Subsystem/KOLoadSubsystem.h"

void UKOSkillSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// LoadSubsystem은 GameInstanceSubsystem이므로 LocalPlayer 생성 시점에 이미 유효함
	if (UGameInstance* GI = GetLocalPlayer()->GetGameInstance())
	{
		CachedLoadSubsystem = GI->GetSubsystem<UKOLoadSubsystem>();
	}

	InitializeSkillStates();
}

void UKOSkillSubsystem::Deinitialize()
{
	CachedLoadSubsystem = nullptr;
	CachedInventoryComponent = nullptr;
	CachedASC = nullptr;

	Super::Deinitialize();
}

UKOSkillSubsystem* UKOSkillSubsystem::Get(const UObject* WorldContext)
{
	if (!WorldContext) return nullptr;

	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) return nullptr;

	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return nullptr;

	ULocalPlayer* LP = GI->GetFirstGamePlayer();
	return LP ? LP->GetSubsystem<UKOSkillSubsystem>() : nullptr;
}

void UKOSkillSubsystem::TryResolveCaches()
{
	if (CachedInventoryComponent && CachedASC)
	{
		return;
	}

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP) return;

	APlayerController* PC = LP->GetPlayerController(GetWorld());
	if (!PC) return;

	if (!CachedInventoryComponent)
	{
		CachedInventoryComponent = PC->FindComponentByClass<UKOInventoryComponent>();
	}

	if (!CachedASC)
	{
		if (AKOPlayerState* PS = PC->GetPlayerState<AKOPlayerState>())
		{
			CachedASC = PS->GetAbilitySystemComponent();
		}
	}
}

bool UKOSkillSubsystem::TryUnlockSkill(const FName& SkillName)
{
	TryResolveCaches();

	if (SkillName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: 스킬명이 없습니다."));
		return false;
	}
	if (IsUnlocked(SkillName))
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: 스킬이 이미 해금된 상태입니다."));
		return false;
	}

	if (CachedLoadSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: 캐싱된 LoadSubsystem이 없습니다."));
		return false;
	}

	if (CachedASC == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: 캐싱된 AbilitySystemComponent가 없습니다."));
		return false;
	}

	const FKOSkillRow* Row = CachedLoadSubsystem->FindSkillRow(SkillName);
	const FKOSkillExecutionRow* ExRow = CachedLoadSubsystem->FindSkillExecutionRow(SkillName);

	if (Row == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: [%s] 스킬 데이터를 찾을 수 없습니다."), *SkillName.ToString());
		return false;
	}

	if (ExRow == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: [%s] 스킬 실행 데이터를 찾을 수 없습니다."), *SkillName.ToString());
		return false;
	}

	if (ArePrerequisitesMet(*Row) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: 선행 조건을 미충족 했습니다."));
		return false;
	}

	ESkillState* State = SkillStates.Find(SkillName);
	if (State == nullptr || *State != ESkillState::CanUnlock)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: SkillState가 없거나 CanUnlock이 아닙니다."));
		return false;
	}

	if (!Row->UnlockCosts.IsEmpty())
	{
		if (CachedInventoryComponent == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: 비용이 있는 스킬인데 InventoryComponent가 없습니다."));
			return false;
		}

		for (const FSkillCost& Cost : Row->UnlockCosts)
		{
			const FName ItemId = CachedLoadSubsystem->FindItemIdByTag(Cost.ItemTag);
			if (ItemId.IsNone())
			{
				UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: 비용 태그 [%s]에 해당하는 아이템이 없습니다."),
				       *Cost.ItemTag.ToString());
				return false;
			}
			if (!CachedInventoryComponent->HasEnoughItems(ItemId, Cost.Amount))
			{
				UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: [%s] 수량 부족 (필요: %d)"),
				       *ItemId.ToString(), Cost.Amount);
				return false;
			}
		}
	}

	FGameplayEffectContextHandle EffectContext = CachedASC->MakeEffectContext();
	
	if (ExRow->ExecutionType == ESkillExecutionType::Active && ExRow->AbilityClass)
	{
		if (CachedASC->FindAbilitySpecFromClass(ExRow->AbilityClass))
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: [%s] 이미 부여된 어빌리티입니다."), *SkillName.ToString());
			return false;
		}

		FGameplayAbilitySpec NewSpec(ExRow->AbilityClass, 1, -1);
		if (ExRow->InputTag.IsValid())
		{
			NewSpec.DynamicAbilityTags.AddTag(ExRow->InputTag);
		}

		CachedASC->GiveAbility(NewSpec);
		UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: [%s] 어빌리티 부여 완료 (InputTag: %s)"),
			*SkillName.ToString(), *ExRow->InputTag.ToString());
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
	
	if (!Row->UnlockCosts.IsEmpty())
	{
		for (const FSkillCost& Cost : Row->UnlockCosts)
		{
			CachedInventoryComponent->TryRemoveItem(CachedLoadSubsystem->FindItemIdByTag(Cost.ItemTag), Cost.Amount);
		}
	}

	*State = ESkillState::Unlocked;
	ReevaluateAllSkillStates();
	return true;
}

ESkillState UKOSkillSubsystem::GetSkillState(const FName& SkillName) const
{
	const ESkillState* State = GetSkillInfo(SkillName);
	return State ? *State : ESkillState::Locked;
}

bool UKOSkillSubsystem::IsUnlocked(const FName& SkillName) const
{
	return GetSkillState(SkillName) == ESkillState::Unlocked;
}

bool UKOSkillSubsystem::HasActiveAbilityInASC(TSubclassOf<UGameplayAbility> AbilityClass) const
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

FGameplayTagContainer UKOSkillSubsystem::GetUnlockedSkillTags() const
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

void UKOSkillSubsystem::GetAllSkillNames(TArray<FName>& Out) const
{
	SkillStates.GetKeys(Out);
}

void UKOSkillSubsystem::InitializeSkillStates()
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

void UKOSkillSubsystem::ReevaluateAllSkillStates()
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
			continue;
		}

		const FKOSkillRow* Row = CachedLoadSubsystem->FindSkillRow(Pair.Key);
		if (Row == nullptr)
		{
			continue;
		}

		Pair.Value = ArePrerequisitesMet(*Row) ? ESkillState::CanUnlock : ESkillState::Locked;
	}
}

bool UKOSkillSubsystem::ArePrerequisitesMet(const FKOSkillRow& Row) const
{
	if (Row.PrerequisiteSkillTags.IsEmpty())
	{
		return true;
	}

	return GetUnlockedSkillTags().HasAll(Row.PrerequisiteSkillTags);
}

const ESkillState* UKOSkillSubsystem::GetSkillInfo(FName SkillName) const
{
	return SkillStates.Find(SkillName);
}