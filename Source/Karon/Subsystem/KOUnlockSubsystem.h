#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "KOUnlockSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FKOUnlockTagGranted, FGameplayTag);

UCLASS()
class KARON_API UKOUnlockSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	static UKOUnlockSubsystem* Get(const UObject* WorldContextObject);

	/** 새로운 해금 태그를 추가한다. 이미 있으면 false 반환 */
	bool GrantUnlockTag(FGameplayTag UnlockTag);

	bool HasUnlockTag(FGameplayTag UnlockTag) const;

	bool HasAllUnlockTags(const FGameplayTagContainer& RequiredTags) const;

	const FGameplayTagContainer& GetOwnedUnlockTags() const
	{
		return OwnedUnlockTags;
	}

	/** 세이브 데이터 복구용 */
	void RestoreUnlockTags(const FGameplayTagContainer& InUnlockTags);

	/** 새 게임 시작용 */
	void ResetUnlockTags();

	FKOUnlockTagGranted OnUnlockTagGranted;

private:
	UPROPERTY()
	FGameplayTagContainer OwnedUnlockTags;
};