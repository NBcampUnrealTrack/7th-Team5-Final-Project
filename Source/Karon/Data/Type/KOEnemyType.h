// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KOEnemyType.generated.h"

/**
 * 
 */
// 스탯 구조체
USTRUCT(BlueprintType)
struct FEnemyStat
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxHP = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float Def = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float Atk = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MoveSpeed = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float DetectRange = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AtkRange = 0.f;
};

USTRUCT(BlueprintType)
struct FEnemySkillInfoTag
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag EnemyNameTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SkillTag;
	
	bool operator==(const FEnemySkillInfoTag& Other) const
	{
		return EnemyNameTag == Other.EnemyNameTag && SkillTag == Other.SkillTag;
	}
};
//구조체가 키여서 GetTypeHash를 오버라이드
FORCEINLINE uint32 GetTypeHash(const FEnemySkillInfoTag& Key)
{
	return HashCombine(GetTypeHash(Key.EnemyNameTag), GetTypeHash(Key.SkillTag));
}