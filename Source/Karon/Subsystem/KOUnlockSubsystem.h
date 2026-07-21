#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KOUnlockSubsystem.generated.h"

class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FKOUnlockTagGranted, FGameplayTag);

UCLASS()
class KARON_API UKOUnlockSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UKOUnlockSubsystem* Get(const UObject* WorldContextObject);

	bool GrantUnlockTag(FGameplayTag UnlockTag);

	bool HasUnlockTag(FGameplayTag UnlockTag) const;

	bool HasAllUnlockTags(const FGameplayTagContainer& RequiredTags) const;

	FGameplayTagContainer GetOwnedUnlockTags() const;

	// 세이브 로드
	void RestoreUnlockTags(const FGameplayTagContainer& InUnlockTags);
	void ResetUnlockTags();

	FKOUnlockTagGranted OnUnlockTagGranted;

private:
	UAbilitySystemComponent* GetPlayerASC() const;
};