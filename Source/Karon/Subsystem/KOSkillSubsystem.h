// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Utility/Messaging/KOGMSInterface.h"
#include "GameplayEffectTypes.h"
#include "Data/Type/KOSkillTypes.h"
#include "KOSkillSubsystem.generated.h"

struct FKOSkillRow;
class UKOLoadSubsystem;
class UKOInventoryComponent;
class AKOPlayerState;
class UAbilitySystemComponent;
class UGameplayAbility;

UCLASS()
class KARON_API UKOSkillSubsystem : public ULocalPlayerSubsystem, public IKOGMSInterface
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
	
	// 세이브 로드 시스템
	void GetSkillStateForSave(TArray<FName>& OutUnlockedSkillIds) const;
	void LoadSkillStateFromSave(const TArray<FName>& InUnlockedSkillIds);
	
	bool SetSkillQuickSlot(ESkillQuickSlotKey SlotKey, FName SkillName);
	void ClearSkillQuickSlot(ESkillQuickSlotKey SlotKey);
	FName GetSkillQuickSlot(ESkillQuickSlotKey SlotKey) const;

	void GetSkillQuickSlotsForSave(TMap<ESkillQuickSlotKey, FName>& OutQuickSlots) const;
	void LoadSkillQuickSlotsFromSave(const TMap<ESkillQuickSlotKey, FName>& InQuickSlots);

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
	
	TMap<FName, FActiveGameplayEffectHandle> GrantedPassiveEffectHandles;

	bool GrantSkillExecutionFromSaveOrUnlock(const FName& SkillName);
	void RemoveAllGrantedSkillExecutions();
	
	TMap<ESkillQuickSlotKey, FName> SkillQuickSlots;

	bool CanAssignSkillToQuickSlot(FName SkillName) const;
	void BroadcastSkillQuickSlotChanged(ESkillQuickSlotKey SlotKey, FName SkillName);
};