// Copyright Karon Team 5. All Rights Reserved.

#include "KOSkillLibrary.h"
#include "Subsystem/KOLoadSubsystem.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

const UKOLoadSubsystem* UKOSkillLibrary::GetLoadSubsystem(const UObject* WorldContext)
{
	return UKOLoadSubsystem::Get(WorldContext);;
}

const FKOSkillRow* UKOSkillLibrary::GetSkillRow(const UObject* WorldContext, FName SkillId)
{
	const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext);
	if (LS == nullptr)
	{
		return nullptr;
	}
	return LS->FindSkillRow(SkillId);
}

FText UKOSkillLibrary::GetDisplayName(const UObject* WorldContext, FName SkillId)
{
	const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext);
	if (LS == nullptr)
	{
		return FText::GetEmpty();
	}
	
	if (const FKOSkillRow* Row = LS->FindSkillRow(SkillId))
	{
		return Row->DisplayName;
	}
	
	return FText::GetEmpty();
}

UTexture2D* UKOSkillLibrary::GetIcon(const UObject* WorldContext, FName SkillId)
{
	const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext);
	if (LS == nullptr)
	{
		return nullptr;
	}
	
	return LS->ResolveSkillIcon(SkillId);
}

bool UKOSkillLibrary::HasRow(const UObject* WorldContext, FName SkillId)
{
	const UKOLoadSubsystem* LS = GetLoadSubsystem(WorldContext);
	if (LS == nullptr)
	{
		return false;
	}
	
	return LS->FindSkillRow(SkillId) != nullptr;
}
