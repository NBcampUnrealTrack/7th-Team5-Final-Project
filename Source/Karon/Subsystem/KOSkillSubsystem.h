// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Data/Type/KOSkillTypes.h"
#include "KOSkillSubsystem.generated.h"

struct FKOSkillRow;
class UKOLoadSubsystem;
class UKOInventoryComponent;
class AKOPlayerState;
class UAbilitySystemComponent;
class UGameplayAbility;

UCLASS()
class KARON_API UKOSkillSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	static UKOSkillSubsystem* Get(const UObject* WorldContext);

	bool TryUnlockSkill(const FName& SkillName);
	bool IsUnlocked(const FName& SkillName) const;
	bool HasActiveAbilityInASC(TSubclassOf<UGameplayAbility> AbilityClass) const;
	ESkillState GetSkillState(const FName& SkillName) const;
	FGameplayTagContainer GetUnlockedSkillTags() const;
	void GetAllSkillNames(TArray<FName>& Out) const;

private:
	UPROPERTY()
	TObjectPtr<UKOLoadSubsystem> CachedLoadSubsystem;
	UPROPERTY()
	TObjectPtr<UKOInventoryComponent> CachedInventoryComponent;
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> CachedASC;

	TMap<FName, ESkillState> SkillStates;

	void TryResolveCaches();
	void InitializeSkillStates();
	void ReevaluateAllSkillStates();
	bool ArePrerequisitesMet(const FKOSkillRow& Row) const;
	const ESkillState* GetSkillInfo(FName SkillId) const;
};