#include "KOUnlockSubsystem.h"

#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"

UKOUnlockSubsystem* UKOUnlockSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject || !GEngine)
	{
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);

	if (!World)
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UKOUnlockSubsystem>();
}

bool UKOUnlockSubsystem::GrantUnlockTag(FGameplayTag UnlockTag)
{
	if (!UnlockTag.IsValid())
	{
		return false;
	}

	if (OwnedUnlockTags.HasTagExact(UnlockTag))
	{
		return false;
	}

	OwnedUnlockTags.AddTag(UnlockTag);

	UE_LOG(LogTemp, Log, TEXT("[Unlock] 태그 획득: %s"), *UnlockTag.ToString());

	OnUnlockTagGranted.Broadcast(UnlockTag);

	return true;
}

bool UKOUnlockSubsystem::HasUnlockTag(FGameplayTag UnlockTag) const
{
	if (!UnlockTag.IsValid())
	{
		return false;
	}

	return OwnedUnlockTags.HasTagExact(UnlockTag);
}

bool UKOUnlockSubsystem::HasAllUnlockTags(const FGameplayTagContainer& RequiredTags) const
{
	return OwnedUnlockTags.HasAll(RequiredTags);
}

void UKOUnlockSubsystem::RestoreUnlockTags(const FGameplayTagContainer& InUnlockTags)
{
	OwnedUnlockTags = InUnlockTags;
}

void UKOUnlockSubsystem::ResetUnlockTags()
{
	OwnedUnlockTags.Reset();
}