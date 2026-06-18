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
struct FEnemySkillInfo
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag EnemyNameTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag SkillTag;
	
	bool operator==(const FEnemySkillInfo& Other) const
	{
		return EnemyNameTag == Other.EnemyNameTag && SkillTag == Other.SkillTag;
	}
};
//구조체가 키여서 GetTypeHash 오버로딩
FORCEINLINE uint32 GetTypeHash(const FEnemySkillInfo& Key)
{
	return HashCombine(GetTypeHash(Key.EnemyNameTag), GetTypeHash(Key.SkillTag));
}

USTRUCT(BlueprintType)
struct FEnemyNameLevelInfo
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FGameplayTag EnemyNameTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level;
	
	bool operator==(const FEnemyNameLevelInfo& Other) const
	{
		return EnemyNameTag == Other.EnemyNameTag && Level == Other.Level;
	}
};
//구조체가 키여서 GetTypeHash를 오버로딩
FORCEINLINE uint32 GetTypeHash(const  FEnemyNameLevelInfo& Key)
{
	return HashCombine(GetTypeHash(Key.EnemyNameTag), GetTypeHash(Key.Level));
}

USTRUCT(BlueprintType)
struct FEnemyInfo
{
	GENERATED_BODY()
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Health;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float AttackPower;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Defense;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AttackSpeed;
};