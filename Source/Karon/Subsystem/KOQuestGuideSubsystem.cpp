#include "KOQuestGuideSubsystem.h"

#include "Component/Inventory/KOInventoryComponent.h"
#include "Engine/DataTable.h"

UKOQuestGuideSubsystem* UKOQuestGuideSubsystem::Get(const UObject* WorldContext)
{
	if (!WorldContext || !GEngine)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::ReturnNull);

	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	return GameInstance ? GameInstance->GetSubsystem<UKOQuestGuideSubsystem>() : nullptr;
}

void UKOQuestGuideSubsystem::InitializeQuestGuide(UDataTable* InQuestTable, FName InStartQuestId)
{
	QuestTable = InQuestTable;

	if (CurrentQuestId.IsNone())
	{
		CurrentQuestId = InStartQuestId;
		OnQuestChanged.Broadcast(CurrentQuestId);
	}
}

void UKOQuestGuideSubsystem::SetCurrentQuest(FName NewQuestId)
{
	if (CurrentQuestId == NewQuestId)
	{
		return;
	}
	
	CurrentProgress = 0;

	CurrentQuestId = NewQuestId;
	OnQuestChanged.Broadcast(CurrentQuestId);
}

const FKOQuestGuideRow* UKOQuestGuideSubsystem::GetCurrentQuestRow() const
{
	if (!QuestTable || CurrentQuestId.IsNone())
	{
		return nullptr;
	}

	return QuestTable->FindRow<FKOQuestGuideRow>(CurrentQuestId, TEXT("QuestGuide"));
}

bool UKOQuestGuideSubsystem::IsQuestCompleted(FName QuestId) const
{
	if (QuestId.IsNone())
	{
		return false;
	}

	return CompletedQuestIds.Contains(QuestId);
}

void UKOQuestGuideSubsystem::GetCompletedQuestIdsForSave(TArray<FName>& OutCompletedQuestIds) const
{
	OutCompletedQuestIds.Reset();
	OutCompletedQuestIds.Reserve(CompletedQuestIds.Num());

	for (const FName& QuestId : CompletedQuestIds)
	{
		OutCompletedQuestIds.Add(QuestId);
	}
}

void UKOQuestGuideSubsystem::LoadCompletedQuestIdsFromSave(const TArray<FName>& InCompletedQuestIds)
{
	CompletedQuestIds.Reset();

	for (const FName& QuestId : InCompletedQuestIds)
	{
		if (!QuestId.IsNone())
		{
			CompletedQuestIds.Add(QuestId);
		}
	}

	OnQuestChanged.Broadcast(CurrentQuestId);
}

FText UKOQuestGuideSubsystem::GetTitleText() const
{
	if (const FKOQuestGuideRow* Row = GetCurrentQuestRow())
	{
		return Row->Title;
	}

	return FText::GetEmpty();
}

FText UKOQuestGuideSubsystem::GetObjectiveText() const
{
	if (const FKOQuestGuideRow* Row = GetCurrentQuestRow())
	{
		return Row->ObjectiveText;
	}

	return FText::GetEmpty();
}

FText UKOQuestGuideSubsystem::GetDescriptionText() const
{
	if (const FKOQuestGuideRow* Row = GetCurrentQuestRow())
	{
		return Row->DescriptionText;
	}

	return FText::GetEmpty();
}

void UKOQuestGuideSubsystem::NotifyItemCollected(FName ItemId, int32 Count)
{
	if (ItemId.IsNone() || Count <= 0)
	{
		return;
	}

	TryCompleteQuest(EKOQuestCompleteType::CollectItem, ItemId, Count);
}

void UKOQuestGuideSubsystem::NotifyWeaponEquipped(FName WeaponItemId)
{
	if (WeaponItemId.IsNone())
	{
		return;
	}

	TryCompleteQuest(EKOQuestCompleteType::EquipWeapon, WeaponItemId, 1);
}

void UKOQuestGuideSubsystem::NotifyMonsterKilled(FName MonsterId)
{
	TryCompleteQuest(EKOQuestCompleteType::KillMonster, MonsterId, 1);
}

void UKOQuestGuideSubsystem::NotifyBuildQuickSlotAssigned(FName FactoryId)
{
	if (FactoryId.IsNone())
	{
		return;
	}
	
	TryCompleteQuest(EKOQuestCompleteType::AssignBuildQuickSlot, FactoryId);
}

void UKOQuestGuideSubsystem::NotifyBuildingPlaced(FName FactoryId)
{
	TryCompleteQuest(EKOQuestCompleteType::PlaceBuilding, FactoryId);
}

void UKOQuestGuideSubsystem::NotifyFuelInserted(FName FuelItemId, int32 Count)
{
	if (Count <= 0)
	{
		return;
	}

	TryCompleteQuest(EKOQuestCompleteType::InsertFuel, FuelItemId);
}

void UKOQuestGuideSubsystem::NotifyItemCrafted(FName ItemId, int32 Count)
{
	if (Count <= 0)
	{
		return;
	}

	TryCompleteQuest(EKOQuestCompleteType::CraftItem, ItemId);
}

void UKOQuestGuideSubsystem::NotifyRecipeSelected(FName RecipeId)
{
	if (RecipeId.IsNone())
	{
		return;
	}
	
	TryCompleteQuest(EKOQuestCompleteType::SelectRecipe, RecipeId);
}

void UKOQuestGuideSubsystem::NotifyProcessorInputInserted(FName ItemId, int32 Count)
{
	if (ItemId.IsNone() || Count <= 0)
	{
		return;
	}

	TryCompleteQuest(EKOQuestCompleteType::InsertProcessorInput, ItemId);
}

void UKOQuestGuideSubsystem::NotifyConveyorOutputBound(FName OutputItemId)
{
	TryCompleteQuest(EKOQuestCompleteType::BindConveyorOutput, OutputItemId);
}

void UKOQuestGuideSubsystem::NotifyProcessorOutputCollected(FName ItemId, int32 Count)
{
	if (ItemId.IsNone() || Count <= 0)
	{
		return;
	}

	TryCompleteQuest(EKOQuestCompleteType::CollectProcessorOutput, ItemId);
}

void UKOQuestGuideSubsystem::NotifySkillUnlocked(FName SkillId)
{
	TryCompleteQuest(EKOQuestCompleteType::UnlockSkill, SkillId);
}

void UKOQuestGuideSubsystem::NotifySkillAssigned(FName SkillId)
{
	TryCompleteQuest(EKOQuestCompleteType::AssignSkillSlot, SkillId);
}

void UKOQuestGuideSubsystem::NotifyBossDefeated(FName BossId)
{
	TryCompleteQuest(EKOQuestCompleteType::DefeatBoss, BossId);
}

bool UKOQuestGuideSubsystem::CanUnlockSkillByQuest() const
{
	const FKOQuestGuideRow* Row = GetCurrentQuestRow();
	if (!Row)
	{
		return true;
	}

	return Row->bAllowSkillUnlock;
}

void UKOQuestGuideSubsystem::GetQuestGuideStateForSave(FName& OutCurrentQuestId, int32& OutCurrentProgress,
	TArray<FName>& OutCompletedQuestIds) const
{
	OutCurrentQuestId = CurrentQuestId;
	OutCurrentProgress = CurrentProgress;

	GetCompletedQuestIdsForSave(OutCompletedQuestIds);
}

void UKOQuestGuideSubsystem::LoadQuestGuideStateFromSave(FName InCurrentQuestId, int32 InCurrentProgress,
	const TArray<FName>& InCompletedQuestIds)
{
	CompletedQuestIds.Reset();

	for (const FName& QuestId : InCompletedQuestIds)
	{
		if (!QuestId.IsNone())
		{
			CompletedQuestIds.Add(QuestId);
		}
	}

	if (!InCurrentQuestId.IsNone())
	{
		CurrentQuestId = InCurrentQuestId;
	}

	CurrentProgress = FMath::Max(0, InCurrentProgress);

	OnQuestChanged.Broadcast(CurrentQuestId);
}

bool UKOQuestGuideSubsystem::TryCompleteQuest(EKOQuestCompleteType CompleteType, FName TargetId, int32 AddProgress)
{
	const FKOQuestGuideRow* Row = GetCurrentQuestRow();
	if (!Row)
	{
		return false;
	}

	if (Row->CompleteType != CompleteType)
	{
		return false;
	}

	// DT의 TargetId가 비어 있으면 타입만 맞아도 완료
	if (!Row->TargetId.IsNone() && Row->TargetId != TargetId)
	{
		return false;
	}
	
	const int32 RequiredCount = FMath::Max(1, Row->RequiredCount);
	CurrentProgress += FMath::Max(1, AddProgress);

	if (CurrentProgress < RequiredCount)
	{
		OnQuestChanged.Broadcast(CurrentQuestId);
		return false;
	}

	GrantRewards(*Row);
	AdvanceQuest();
	return true;
}

void UKOQuestGuideSubsystem::GrantRewards(const FKOQuestGuideRow& Row)
{
	if (Row.Rewards.Num() <= 0)
	{
		return;
	}

	UKOInventoryComponent* Inventory = GetPlayerInventory();
	if (!Inventory)
	{
		return;
	}

	for (const FKOQuestReward& Reward : Row.Rewards)
	{
		if (Reward.ItemId.IsNone() || Reward.Count <= 0)
		{
			continue;
		}

		Inventory->TryAddItem(Reward.SlotKind, Reward.ItemId, Reward.Count);
	}
}

UKOInventoryComponent* UKOQuestGuideSubsystem::GetPlayerInventory() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (!PlayerController)
	{
		return nullptr;
	}

	return PlayerController->FindComponentByClass<UKOInventoryComponent>();
}

void UKOQuestGuideSubsystem::AdvanceQuest()
{
	const FName CompletedQuestId = CurrentQuestId;
	
	const FKOQuestGuideRow* Row = GetCurrentQuestRow();
	if (!Row)
	{
		return;
	}

	if (!CompletedQuestId.IsNone())
	{
		CompletedQuestIds.Add(CompletedQuestId);
	}
	
	if (Row->NextQuestId.IsNone())
	{
		OnQuestChanged.Broadcast(CurrentQuestId);
		return;
	}

	SetCurrentQuest(Row->NextQuestId);
}
