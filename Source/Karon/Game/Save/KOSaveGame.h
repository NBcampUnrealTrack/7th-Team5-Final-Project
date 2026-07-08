#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Items/KOItemSlot.h"
#include "Data/Type/KOSkillTypes.h"
#include "Component/Inventory/KOEquipmentComponent.h"
#include "KOSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FKOSavedProcessorState // 설비 내부 아이템
{
	GENERATED_BODY()

	UPROPERTY()
	FName SelectedRecipeId = NAME_None;

	UPROPERTY()
	TMap<FName, int32> InputBuffer;

	UPROPERTY()
	TMap<FName, int32> OutputBuffer;
	
	// 제작 진행률
	UPROPERTY()
	FName ActiveRecipeId = NAME_None;

	UPROPERTY()
	float CurrentCycleSeconds = 0.f;

	UPROPERTY()
	float Progress = 0.f;
};

USTRUCT(BlueprintType)
struct FKOSavedProducerState // 압력
{
	GENERATED_BODY()

	UPROPERTY()
	FName FuelItemId = NAME_None;

	UPROPERTY()
	int32 FuelCount = 0;

	UPROPERTY()
	float FuelDebt = 0.f;
};

USTRUCT(BlueprintType)
struct FKOSavedConveyorState // 컨베이어 벨트
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FName> SlotItemIds;

	// 슬롯 사이 이동 진행값
	UPROPERTY()
	float MoveAccumulator = 0.f;

	// L자형 벨트 방향 (아이템 흐름)
	UPROPERTY()
	bool bCornerFlip = false;

	// 직선 벨트 역방향 여부 (아이템 흐름)
	UPROPERTY()
	bool bStraightReverse = false;
	
	// Output 포트 바인딩 저장
	UPROPERTY()
	bool bHasOutputBinding = false;

	UPROPERTY()
	FIntPoint BoundOutputMachineGridAnchor = FIntPoint::ZeroValue;

	UPROPERTY()
	int32 BoundOutputPortIndex = INDEX_NONE;

	UPROPERTY()
	FName BoundOutputItemId = NAME_None;

	UPROPERTY()
	bool bHasSelectedOutputPort = false;
};

USTRUCT(BlueprintType)
struct FKOSavedBuilding // 설비
{
	GENERATED_BODY()

	UPROPERTY()
	FName FactoryId = NAME_None;

	UPROPERTY()
	FTransform Transform = FTransform::Identity;
	
	UPROPERTY()
	FIntPoint GridAnchor = FIntPoint::ZeroValue;

	UPROPERTY()
	FIntPoint GridSize = FIntPoint(1, 1);

	UPROPERTY()
	FKOSavedProcessorState ProcessorState;

	UPROPERTY()
	FKOSavedProducerState ProducerState;
	
	// 컨베이어 벨트
	UPROPERTY()
	FKOSavedConveyorState ConveyorState;
};

USTRUCT(BlueprintType)
struct FKOSavedSkillState // 스킬
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FName> UnlockedSkillIds;
	
	UPROPERTY()
	TMap<ESkillQuickSlotKey, FName> SkillQuickSlots;
};

USTRUCT(BlueprintType)
struct FKOSavedQuestGuideState // 퀘스트 진행도
{
	GENERATED_BODY()

	UPROPERTY()
	FName CurrentQuestId = NAME_None;

	UPROPERTY()
	int32 CurrentProgress = 0;

	UPROPERTY()
	TArray<FName> CompletedQuestIds;
};

USTRUCT(BlueprintType)
struct FKOSavedPlayerStatus // 플레이어 상태
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHasHealth = false;

	UPROPERTY()
	float Health = 0.f;
};

USTRUCT(BlueprintType)
struct FKOSavedMonster // 몬스터 상태
{
	GENERATED_BODY()

	UPROPERTY()
	FName MonsterSaveId = NAME_None;

	UPROPERTY()
	FName ClusterSaveId = NAME_None;

	UPROPERTY()
	FSoftClassPath MonsterClassPath;

	UPROPERTY()
	FTransform Transform = FTransform::Identity;

	UPROPERTY()
	int32 Level = 1;
};

USTRUCT(BlueprintType)
struct FKOSavedBoss // 보스 상태
{
	GENERATED_BODY()

	UPROPERTY()
	FName BossSaveId = NAME_None;

	UPROPERTY()
	FSoftClassPath BossClassPath;

	UPROPERTY()
	FTransform Transform = FTransform::Identity;

	UPROPERTY()
	bool bWasAlive = true;
};

UCLASS()
class KARON_API UKOSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 플레이어 위치
	UPROPERTY()
	bool bHasPlayerTransform = false;

	UPROPERTY()
	FTransform PlayerTransform = FTransform::Identity;
	
	// 플레이어 상태
	UPROPERTY()
	FKOSavedPlayerStatus PlayerStatus;

	// 인벤토리
	UPROPERTY()
	TArray<FKOItemSlot> InventorySlots;

	// 장착 무기
	UPROPERTY()
	FName EquippedWeaponItemId = NAME_None;

	UPROPERTY()
	EWeaponSlot EquippedWeaponSlot = EWeaponSlot::Holster;
	
	// 장착 방어구
	UPROPERTY()
	TMap<EKOEquipmentSlotType, FName> EquippedArmorItemIds;
	
	// 건설 퀵슬롯
	UPROPERTY()
	TArray<FName> BuildQuickSlots;
	
	// 설비
	UPROPERTY()
	TArray<FKOSavedBuilding> Buildings;
	
	// 스킬 해금 상태
	UPROPERTY()
	FKOSavedSkillState SkillState;
	
	// 퀘스트 진행도
	UPROPERTY()
	FKOSavedQuestGuideState QuestGuideState;
	
	// 몬스터 상태
	UPROPERTY()
	TArray<FKOSavedMonster> Monsters;

	UPROPERTY()
	TArray<FName> DeadMonsterIds;
	
	// 보스 상태
	UPROPERTY()
	TArray<FKOSavedBoss> Bosses;
	
	// 이미 채집된 맵 채집물 ID
	UPROPERTY()
	TArray<FName> CollectedItemDropIds;	
};