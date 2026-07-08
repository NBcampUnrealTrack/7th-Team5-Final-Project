#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/Type/KOQuestGuideTypes.h"
#include "KOQuestGuideSubsystem.generated.h"

class UDataTable;
class UKOInventoryComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKOQuestGuideChangedSignature, FName, NewQuestId);

UCLASS()
class KARON_API UKOQuestGuideSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UKOQuestGuideSubsystem* Get(const UObject* WorldContext);
	void InitializeQuestGuide(UDataTable* InQuestTable, FName InStartQuestId);

	void SetCurrentQuest(FName NewQuestId);

	FName GetCurrentQuestId() const { return CurrentQuestId; }

	const FKOQuestGuideRow* GetCurrentQuestRow() const;
	
	bool IsQuestCompleted(FName QuestId) const;

	void GetCompletedQuestIdsForSave(TArray<FName>& OutCompletedQuestIds) const;
	void LoadCompletedQuestIdsFromSave(const TArray<FName>& InCompletedQuestIds);

	FText GetTitleText() const;
	FText GetObjectiveText() const;
	FText GetDescriptionText() const;

	void NotifyItemCollected(FName ItemId, int32 Count); // 아이템 획득
	void NotifyWeaponEquipped(FName WeaponItemId); // 무기 장착
	void NotifyMonsterKilled(FName MonsterId); // 몬스터 처치
	void NotifyBuildQuickSlotAssigned(FName FactoryId); // 설비 슬롯에 설비 할당
	void NotifyBuildingPlaced(FName FactoryId); // 설비 건설
	void NotifyFuelInserted(FName FuelItemId, int32 Count); // 연료 슬롯 할당 (보일러)
	void NotifyItemCrafted(FName ItemId, int32 Count); // 설비 제작
	void NotifyRecipeSelected(FName RecipeId); // 레시피 선택
	void NotifyProcessorInputInserted(FName ItemId, int32 Count); // input 슬롯 할당
	void NotifyConveyorOutputBound(FName OutputItemId); // 벨트 output 슬롯 할당
	void NotifyProcessorOutputCollected(FName ItemId, int32 Count); // Output 슬롯 아이템 인벤토리 수령
	void NotifySkillUnlocked(FName SkillId); // 스킬 해금
	void NotifySkillAssigned(FName SkillId); // 스킬 슬롯 할당
	void NotifyBossDefeated(FName BossId); // 보스 처치
	
	bool CanUnlockSkillByQuest() const; // 스킬 해금 가능 여부

	FKOQuestGuideChangedSignature OnQuestChanged;
	
	// 세이브 로드
	void GetQuestGuideStateForSave(
		FName& OutCurrentQuestId,
		int32& OutCurrentProgress,
		TArray<FName>& OutCompletedQuestIds
	) const;

	void LoadQuestGuideStateFromSave(
		FName InCurrentQuestId,
		int32 InCurrentProgress,
		const TArray<FName>& InCompletedQuestIds
	);

private:
	bool TryCompleteQuest(EKOQuestCompleteType CompleteType, FName TargetId, int32 AddProgress = 1);
	void GrantRewards(const FKOQuestGuideRow& Row); // 퀘스트 보상
	UKOInventoryComponent* GetPlayerInventory() const;
	void AdvanceQuest();

	UPROPERTY()
	TObjectPtr<UDataTable> QuestTable = nullptr;

	UPROPERTY()
	FName CurrentQuestId = NAME_None;
	
	UPROPERTY()
	int32 CurrentProgress = 0;
	
	UPROPERTY()
	TSet<FName> CompletedQuestIds;
};