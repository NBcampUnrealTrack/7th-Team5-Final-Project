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
