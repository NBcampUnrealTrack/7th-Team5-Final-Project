// Copyright Karon Team 5. All Rights Reserved.

#include "KOSkillSubsystem.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Tag/Data/KOGameplayTags_Data.h"
#include "Subsystem/KOQuestGuideSubsystem.h"
#include "Data/Type/KOSkillTypes.h"
#include "Game/KOPlayerState.h"
#include "Component/Inventory/KOInventoryComponent.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Utility/Messaging/KOMessageTypes.h"
#include "StructUtils/InstancedStruct.h"


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
	GrantedPassiveEffectHandles.Empty();
	
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

	LastUnlockFailureReason = FText::GetEmpty();
	
	if (SkillName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: 스킬명이 없습니다."));
		return false;
	}
	if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
	{
		if (!QuestGuide->CanUnlockSkillByQuest())
		{
			LastUnlockFailureReason = FText::FromString(
				TEXT("아직 스킬을 해금할 수 없습니다.\n튜토리얼을 먼저 진행하세요.")
			);
			
			UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: 아직 스킬 해금이 허용된 튜토리얼 단계가 아닙니다."));
			return false;
		}
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
	
	// 아이템 소비 전, 스킬 실행 부여 가능 여부 확인
	if (!GrantSkillExecutionFromSaveOrUnlock(SkillName))
	{
		return false;
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
	
	// 퀘스트
	if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
	{
		QuestGuide->NotifySkillUnlocked(SkillName);
	}
	
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

void UKOSkillSubsystem::GetSkillStateForSave(TArray<FName>& OutUnlockedSkillIds) const
{
	OutUnlockedSkillIds.Reset();

	for (const TPair<FName, ESkillState>& Pair : SkillStates)
	{
		if (Pair.Value == ESkillState::Unlocked)
		{
			OutUnlockedSkillIds.Add(Pair.Key);
		}
	}
}

void UKOSkillSubsystem::LoadSkillStateFromSave(const TArray<FName>& InUnlockedSkillIds)
{
	TryResolveCaches();

	if (!CachedLoadSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: 스킬 로드 실패 - LoadSubsystem 없음"));
		return;
	}

	if (!CachedASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: 스킬 로드 실패 - ASC 없음"));
		return;
	}

	// 기존에 부여된 스킬 효과/어빌리티를 정리한다.
	// 로드 전 런타임에서 다른 스킬을 해금했다가 이전 세이브를 로드하는 경우를 방지.
	RemoveAllGrantedSkillExecutions();

	// DT 기준으로 전체 스킬 상태 초기화
	InitializeSkillStates();

	TSet<FName> UniqueUnlockedSkillIds;

	for (const FName& SkillId : InUnlockedSkillIds)
	{
		if (SkillId.IsNone())
		{
			continue;
		}

		if (!CachedLoadSubsystem->FindSkillRow(SkillId))
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: 저장된 스킬 ID가 DT_Skill에 없습니다. SkillId=%s"), *SkillId.ToString());
			continue;
		}

		UniqueUnlockedSkillIds.Add(SkillId);
	}

	for (const FName& SkillId : UniqueUnlockedSkillIds)
	{
		ESkillState* State = SkillStates.Find(SkillId);
		if (!State)
		{
			continue;
		}

		*State = ESkillState::Unlocked;
		GrantSkillExecutionFromSaveOrUnlock(SkillId);
	}

	ReevaluateAllSkillStates();

	UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: 스킬 로드 완료. UnlockedCount=%d"), UniqueUnlockedSkillIds.Num());
}

bool UKOSkillSubsystem::SetSkillQuickSlot(ESkillQuickSlotKey SlotKey, FName SkillName)
{
	TryResolveCaches();

	if (!SkillName.IsNone() && !CanAssignSkillToQuickSlot(SkillName))
	{
		UE_LOG(
			LogTemp,
			Warning,
			TEXT("SkillSubsystem: 퀵슬롯에 배정할 수 없는 스킬입니다. Skill=%s"),
			*SkillName.ToString()
		);
		return false;
	}
	
	// 같은 스킬이 들어 있는 다른 슬롯을 찾는다.
	TArray<ESkillQuickSlotKey> DuplicateSlotKeys;

	if (!SkillName.IsNone())
	{
		for (const TPair<ESkillQuickSlotKey, FName>& Pair : SkillQuickSlots)
		{
			if (Pair.Key == SlotKey)
			{
				continue;
			}

			if (Pair.Value == SkillName)
			{
				DuplicateSlotKeys.Add(Pair.Key);
			}
		}
	}

	// 다른 슬롯에 같은 스킬이 있으면 먼저 해제한다.
	for (const ESkillQuickSlotKey DuplicateSlotKey : DuplicateSlotKeys)
	{
		FName& DuplicateSkillName = SkillQuickSlots.FindOrAdd(DuplicateSlotKey);

		DuplicateSkillName = NAME_None;

		BroadcastSkillQuickSlotChanged(DuplicateSlotKey, NAME_None);
	}

	FName& CurrentSkillName = SkillQuickSlots.FindOrAdd(SlotKey);

	if (CurrentSkillName == SkillName)
	{
		return true;
	}

	CurrentSkillName = SkillName;

	BroadcastSkillQuickSlotChanged(SlotKey, SkillName);

	// 퀘스트
	if (!SkillName.IsNone())
	{
		if (UKOQuestGuideSubsystem* QuestGuide = UKOQuestGuideSubsystem::Get(this))
		{
			QuestGuide->NotifySkillAssigned(SkillName);
		}
	}
	
	return true;
}

void UKOSkillSubsystem::ClearSkillQuickSlot(ESkillQuickSlotKey SlotKey)
{
	SetSkillQuickSlot(SlotKey, NAME_None);
}

FName UKOSkillSubsystem::GetSkillQuickSlot(ESkillQuickSlotKey SlotKey) const
{
	const FName* FoundSkillName = SkillQuickSlots.Find(SlotKey);
	return FoundSkillName ? *FoundSkillName : NAME_None;
}

void UKOSkillSubsystem::GetSkillQuickSlotsForSave(TMap<ESkillQuickSlotKey, FName>& OutQuickSlots) const
{
	OutQuickSlots = SkillQuickSlots;
}

void UKOSkillSubsystem::LoadSkillQuickSlotsFromSave(const TMap<ESkillQuickSlotKey, FName>& InQuickSlots)
{
	SkillQuickSlots.Empty();

	const ESkillQuickSlotKey SlotKeys[] =
	{
		ESkillQuickSlotKey::Q,
		ESkillQuickSlotKey::E,
		ESkillQuickSlotKey::R,
		ESkillQuickSlotKey::V
	};

	for (const ESkillQuickSlotKey SlotKey : SlotKeys)
	{
		FName SkillName = NAME_None;

		if (const FName* SavedSkillName = InQuickSlots.Find(SlotKey))
		{
			SkillName = *SavedSkillName;
		}

		if (!SkillName.IsNone() && !CanAssignSkillToQuickSlot(SkillName))
		{
			UE_LOG(
				LogTemp,
				Warning,
				TEXT("SkillSubsystem: 저장된 퀵슬롯 스킬을 복원할 수 없습니다. Slot=%d, Skill=%s"),
				static_cast<int32>(SlotKey),
				*SkillName.ToString()
			);

			SkillName = NAME_None;
		}

		SkillQuickSlots.Add(SlotKey, SkillName);
		BroadcastSkillQuickSlotChanged(SlotKey, SkillName);
	}
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

bool UKOSkillSubsystem::GrantSkillExecutionFromSaveOrUnlock(const FName& SkillName)
{
	if (!CachedLoadSubsystem || !CachedASC)
	{
		return false;
	}

	const FKOSkillExecutionRow* ExRow = CachedLoadSubsystem->FindSkillExecutionRow(SkillName);
	if (!ExRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: [%s] 스킬 실행 데이터를 찾을 수 없습니다."), *SkillName.ToString());
		return false;
	}

	if (ExRow->ExecutionType == ESkillExecutionType::Active && ExRow->AbilityClass)
	{
		if (CachedASC->FindAbilitySpecFromClass(ExRow->AbilityClass))
		{
			return true;
		}

		FGameplayAbilitySpec NewSpec(ExRow->AbilityClass, 1, -1);

		if (ExRow->InputTag.IsValid())
		{
			NewSpec.GetDynamicSpecSourceTags().AddTag(ExRow->InputTag);
		}

		CachedASC->GiveAbility(NewSpec);

		UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: [%s] 어빌리티 부여 완료"), *SkillName.ToString());
		return true;
	}

	if (ExRow->ExecutionType == ESkillExecutionType::PassiveStat && ExRow->PassiveEffectClass)
	{
		if (const FActiveGameplayEffectHandle* ExistingHandle = GrantedPassiveEffectHandles.Find(SkillName))
		{
			if (ExistingHandle->IsValid())
			{
				return true;
			}
		}

		FGameplayEffectContextHandle EffectContext = CachedASC->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = CachedASC->MakeOutgoingSpec(
			ExRow->PassiveEffectClass,
			1.f,
			EffectContext
		);

		if (!SpecHandle.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("SkillSubsystem: [%s] Passive SpecHandle 생성 실패"), *SkillName.ToString());
			return false;
		}

		const FActiveGameplayEffectHandle EffectHandle =
			CachedASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

		if (EffectHandle.IsValid())
		{
			GrantedPassiveEffectHandles.Add(SkillName, EffectHandle);
		}

		for (FGameplayAbilitySpec& AbilitySpec : CachedASC->GetActivatableAbilities())
		{
			CachedASC->MarkAbilitySpecDirty(AbilitySpec);
		}

		UE_LOG(LogTemp, Log, TEXT("SkillSubsystem: [%s] 패시브 효과 부여 완료"), *SkillName.ToString());
		return true;
	}

	return true;
}

void UKOSkillSubsystem::RemoveAllGrantedSkillExecutions()
{
	if (!CachedLoadSubsystem || !CachedASC)
	{
		GrantedPassiveEffectHandles.Empty();
		return;
	}

	TArray<FName> AllSkillIds;
	CachedLoadSubsystem->GetAllSkillIds(AllSkillIds);

	for (const FName& SkillId : AllSkillIds)
	{
		const FKOSkillExecutionRow* ExRow = CachedLoadSubsystem->FindSkillExecutionRow(SkillId);
		if (!ExRow)
		{
			continue;
		}

		if (ExRow->ExecutionType == ESkillExecutionType::Active && ExRow->AbilityClass)
		{
			if (FGameplayAbilitySpec* Spec = CachedASC->FindAbilitySpecFromClass(ExRow->AbilityClass))
			{
				CachedASC->ClearAbility(Spec->Handle);
			}
		}
	}

	for (const TPair<FName, FActiveGameplayEffectHandle>& Pair : GrantedPassiveEffectHandles)
	{
		if (Pair.Value.IsValid())
		{
			CachedASC->RemoveActiveGameplayEffect(Pair.Value);
		}
	}

	GrantedPassiveEffectHandles.Empty();
}

bool UKOSkillSubsystem::CanAssignSkillToQuickSlot(FName SkillName) const
{
	if (SkillName.IsNone())
	{
		return true;
	}

	if (!IsUnlocked(SkillName))
	{
		return false;
	}

	if (!CachedLoadSubsystem)
	{
		return false;
	}

	const FKOSkillExecutionRow* ExRow =
		CachedLoadSubsystem->FindSkillExecutionRow(SkillName);

	if (!ExRow)
	{
		return false;
	}

	return ExRow->ExecutionType == ESkillExecutionType::Active;
}

void UKOSkillSubsystem::BroadcastSkillQuickSlotChanged(ESkillQuickSlotKey SlotKey, FName SkillName)
{
	FKOSkillQuickSlotChangedMessage Msg;
	Msg.SlotKey = SlotKey;
	Msg.SkillName = SkillName;
	Msg.SkillTag = FGameplayTag::EmptyTag;

	if (!SkillName.IsNone() && CachedLoadSubsystem)
	{
		if (const FKOSkillRow* SkillRow = CachedLoadSubsystem->FindSkillRow(SkillName))
		{
			Msg.SkillTag = SkillRow->SkillTag;
		}
	}

	Broadcast(
		KOGameplayTags::Data_Message_Skill_QuickSlotChanged,
		FInstancedStruct::Make<FKOSkillQuickSlotChangedMessage>(Msg)
	);
}
