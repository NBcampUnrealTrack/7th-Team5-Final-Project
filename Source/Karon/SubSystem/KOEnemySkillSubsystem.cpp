// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemySkillSubsystem.h"

#include "GameplayTagContainer.h"
#include "Data/KODataTableTypes.h"
#include "Data/Character/Enemy/KOEnemyDeveloperSettings.h"
#include "Data/Type/KOEnemyType.h"

UKOEnemySkillSubsystem* UKOEnemySkillSubsystem::Get(UObject* WorldContext)
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
    
	return GI->GetSubsystem<UKOEnemySkillSubsystem>();
}

void UKOEnemySkillSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
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
				FEnemySkillInfoTag EnemyTag;
				EnemyTag.EnemyNameTag=Value.EnemyNameTag;
				EnemyTag.SkillTag=Value.SkillTag;
				SkillDataMap.Add(EnemyTag,Value.SkillMultiplier);
			});
	}
}

float UKOEnemySkillSubsystem::GetSkillData(FEnemySkillInfoTag EnemyTag)
{
	if (const float* Value = SkillDataMap.Find(EnemyTag))
	{
		return *Value;
	}
	return 0.f;
}
