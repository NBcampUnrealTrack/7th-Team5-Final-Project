#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Items/KOItemSlot.h"
#include "KOQuestGuideTypes.generated.h"

UENUM(BlueprintType)
enum class EKOQuestCompleteType : uint8
{
	None,
	// 아이템 획득 (채집)
	CollectItem,
	// 무기 장착
	EquipWeapon,
	// 몬스터 처치
	KillMonster,
	// 설비 슬롯에 설비 할당
	AssignBuildQuickSlot,
	// 설비 설치
	PlaceBuilding,
	// 석탄 넣기
	InsertFuel,	
	// 설비 제작
	CraftItem,			
	// 레시피 선택
	SelectRecipe,		
	// input 아이템 넣기
	InsertProcessorInput,	
	// 벨트 output 슬롯 할당
	BindConveyorOutput,
	// 스킬 해금
	UnlockSkill,
	// 스킬 슬롯 할당
	AssignSkillSlot,
	// 보스 처치
	DefeatBoss,
	// 화톳불 활성화
	ActivateBonfire,
	// 화톳불을 통한 이동
	TeleportBonfire
};

USTRUCT(BlueprintType)
struct KARON_API FKOQuestReward
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EKOSlotKind SlotKind = EKOSlotKind::Factory;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Count = 1;
};

USTRUCT(BlueprintType)
struct KARON_API FKOQuestGuideRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ObjectiveText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DescriptionText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EKOQuestCompleteType CompleteType = EKOQuestCompleteType::None;

	/* 해당 타겟 ID */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TargetId = NAME_None;

	/* 다음 퀘스트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName NextQuestId = NAME_None;
	
	/* 퀘스트 보상 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FKOQuestReward> Rewards;
	
	/* 몬스터 처치 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequiredCount = 1;
	
	/* 스킬 해금 가능 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bAllowSkillUnlock = false;
};