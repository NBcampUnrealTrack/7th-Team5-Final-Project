// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "KOSkillTypes.generated.h"

UENUM(BlueprintType)
enum class ESkillState : uint8
{
	Locked,      // 잠김 (선행 스킬 미달 등)
	CanUnlock,   // 해금 가능
	Unlocked     // 습득 완료
};

UENUM(BlueprintType)
enum class ESkillExecutionType : uint8
{
	Active,			 // 발동 스킬 추가
	ActiveExtension, // 행동 추가
	PassiveStat		 // 스탯 증가
};

UENUM(BlueprintType)
enum class ESkillQuickSlotKey : uint8
{
	Q UMETA(DisplayName = "Q"),
	E UMETA(DisplayName = "E"),
	R UMETA(DisplayName = "R"),
	V UMETA(DisplayName = "V"),
};

USTRUCT(BlueprintType)
struct FSkillCost
{
	GENERATED_BODY()

	// 어떤 아이템을 소모할 것인가?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FGameplayTag ItemTag;

	// 몇 개를 소모할 것인가?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	int32 Amount = 0;
};
