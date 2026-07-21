#include "KOUnlockSubsystem.h"

#include "AbilitySystemComponent.h"
#include "Game/KOPlayerController.h"
#include "Game/KOPlayerState.h"
#include "Kismet/GameplayStatics.h"

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

	UAbilitySystemComponent* ASC = GetPlayerASC();
	if (!ASC)
	{
		return false;
	}

	if (ASC->HasMatchingGameplayTag(UnlockTag))
	{
		return false;
	}

	ASC->AddLooseGameplayTag(UnlockTag);

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
	
	const UAbilitySystemComponent* ASC = GetPlayerASC();
	return ASC && ASC->HasMatchingGameplayTag(UnlockTag);
}

bool UKOUnlockSubsystem::HasAllUnlockTags(const FGameplayTagContainer& RequiredTags) const
{
	const UAbilitySystemComponent* ASC = GetPlayerASC();
	return ASC && ASC->HasAllMatchingGameplayTags(RequiredTags);
}

FGameplayTagContainer UKOUnlockSubsystem::GetOwnedUnlockTags() const
{
	FGameplayTagContainer Result;

	const UAbilitySystemComponent* ASC = GetPlayerASC();
	if (!ASC)
	{
		return Result;
	}

	const FGameplayTag UnlockRootTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Unlock.Core")),false);

	if (!UnlockRootTag.IsValid())
	{
		return Result;
	}

	FGameplayTagContainer AllOwnedTags;
	ASC->GetOwnedGameplayTags(AllOwnedTags);

	for (const FGameplayTag& OwnedTag : AllOwnedTags)
	{
		if (OwnedTag.MatchesTag(UnlockRootTag))
		{
			Result.AddTag(OwnedTag);
		}
	}

	return Result;
}

void UKOUnlockSubsystem::RestoreUnlockTags(const FGameplayTagContainer& InUnlockTags)
{
	UAbilitySystemComponent* ASC = GetPlayerASC();
	if (!ASC)
	{
		return;
	}

	ResetUnlockTags();

	for (const FGameplayTag& UnlockTag : InUnlockTags)
	{
		if (!UnlockTag.IsValid())
		{
			continue;
		}

		ASC->AddLooseGameplayTag(UnlockTag);
	}
}

void UKOUnlockSubsystem::ResetUnlockTags()
{
	UAbilitySystemComponent* ASC = GetPlayerASC();
	if (!ASC)
	{
		return;
	}

	const FGameplayTagContainer UnlockTags = GetOwnedUnlockTags();

	for (const FGameplayTag& UnlockTag : UnlockTags)
	{
		ASC->SetLooseGameplayTagCount(UnlockTag, 0);
	}
}

UAbilitySystemComponent* UKOUnlockSubsystem::GetPlayerASC() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	AKOPlayerController* PlayerController = Cast<AKOPlayerController>(
			UGameplayStatics::GetPlayerController(World, 0));

	if (!PlayerController)
	{
		return nullptr;
	}

	AKOPlayerState* PlayerState = PlayerController->GetPlayerState<AKOPlayerState>();

	if (!PlayerState)
	{
		return nullptr;
	}

	return PlayerState->GetAbilitySystemComponent();
}
