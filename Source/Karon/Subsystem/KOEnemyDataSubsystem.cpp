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
	InitEnemyDropItemSettings();
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

TArray<FEnemyDropItemInfo>* UKOEnemyDataSubsystem::GetEnemyDropItemArray(FEnemyNameLevelInfo EnemyNameLevelInfo)
{
	if (FEnemyDropItemArrayWrapper* EnemyDropItemArray = EnemyDropItemMap.Find(EnemyNameLevelInfo))
	{
		return &EnemyDropItemArray->DropItems;
	}
	return nullptr;
}

void UKOEnemyDataSubsystem::InitEnemySkillSettings()
{
	const UKOEnemyDeveloperSettings* DeveloperSettings = GetDefault<UKOEnemyDeveloperSettings>();
	if(!IsValid(DeveloperSettings))
	{
		return;
	}
	
	UDataTable* SkillDT = DeveloperSettings->SkillMulDataTable.LoadSynchronous();
	
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
	const UKOEnemyDeveloperSettings* DeveloperSettings = GetDefault<UKOEnemyDeveloperSettings>();
	if(!IsValid(DeveloperSettings))
	{
		return;
	}
	
	UDataTable* EnemyDT = DeveloperSettings->EnemyDataTable.LoadSynchronous();
	
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

void UKOEnemyDataSubsystem::InitEnemyDropItemSettings()
{
	const UKOEnemyDeveloperSettings* DeveloperSettings = GetDefault<UKOEnemyDeveloperSettings>();
	if(!IsValid(DeveloperSettings))
	{
		return;
	}
	
	UDataTable* EnemyDT = DeveloperSettings->EnemyDropItemDataTable.LoadSynchronous();
	
	if (IsValid(EnemyDT))
	{
		EnemyDT->ForeachRow<FKOEnemyDropItemRow>(TEXT("EDSkillDataSubsystem Init"), 
			[this](const FName& Key, const FKOEnemyDropItemRow& Value)
			{
				const FEnemyNameLevelInfo& EnemyNameLevelInfo=Value.NameLevelData;
				const FEnemyDropItemInfo& DropItemInfo=Value.DropItemInfo;
				
				EnemyDropItemMap.FindOrAdd(EnemyNameLevelInfo).DropItems.Add(DropItemInfo);
			});
	}
}
