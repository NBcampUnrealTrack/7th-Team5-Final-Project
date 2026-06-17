// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyDataSubsystem.h"

#include "GameplayTagContainer.h"
#include "Data/KODataTableTypes.h"
#include "Data/Character/Enemy/KOEnemyDeveloperSettings.h"
#include "Data/Type/KOEnemyType.h"

UKOEnemyDataSubsystem* UKOEnemyDataSubsystem::Get(UObject* WorldContext)
{
	if (!WorldContext||!WorldContext->GetWorld())
	{
		return nullptr;
	}
	UGameInstance* GI = WorldContext->GetWorld()->GetGameInstance();
	if (!GI)
	{
		return nullptr;
	}
    
	return GI->GetSubsystem<UKOEnemyDataSubsystem>();
}

void UKOEnemyDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	InitEnemySkillSettings();
	InitEnemyDataSettings();
}

float UKOEnemyDataSubsystem::GetSkillData(FEnemySkillInfo EnemySkillInfo)
{
	if (const float* Value = SkillDataMap.Find(EnemySkillInfo))
	{
		return *Value;
	}
	return 0.f;
}

FEnemyInfo* UKOEnemyDataSubsystem::GetEnemyData(FEnemyNameLevelInfo EnemyNameLevelInfo)
{
	if (FEnemyInfo* EnemyInfo = EnemyDataMap.Find(EnemyNameLevelInfo))
	{
		return EnemyInfo;
	}
	return nullptr;
}

void UKOEnemyDataSubsystem::InitEnemySkillSettings()
{
	const UKOEnemyDeveloperSettings* SkillSettings = GetDefault<UKOEnemyDeveloperSettings>();
	if(!IsValid(SkillSettings))
	{
		return;
	}
	
	UDataTable* SkillDT = SkillSettings->SkillMulDataTable.LoadSynchronous();
	
	if (IsValid(SkillDT))
	{
		SkillDT->ForeachRow<FKOEnemySkillRow>(TEXT("EDSkillDataSubsystem Init"), 
			[this](const FName& Key, const FKOEnemySkillRow& Value)
			{
				const FEnemySkillInfo& SkillInfo=Value.SkillInfo;
				SkillDataMap.Add(SkillInfo,Value.SkillMultiplier);
			});
	}
}

void UKOEnemyDataSubsystem::InitEnemyDataSettings()
{
	const UKOEnemyDeveloperSettings* SkillSettings = GetDefault<UKOEnemyDeveloperSettings>();
	if(!IsValid(SkillSettings))
	{
		return;
	}
	
	UDataTable* EnemyDT = SkillSettings->EnemyDataTable.LoadSynchronous();
	
	if (IsValid(EnemyDT))
	{
		EnemyDT->ForeachRow<FKOEnemyDataRow>(TEXT("EDSkillDataSubsystem Init"), 
			[this](const FName& Key, const FKOEnemyDataRow& Value)
			{
				const FEnemyNameLevelInfo& EnemyNameLevelInfo=Value.NameLevelData;
				const FEnemyInfo& EnemyInfo=Value.EnemyInfo;

				EnemyDataMap.Add(EnemyNameLevelInfo,EnemyInfo);
			});
	}
}
