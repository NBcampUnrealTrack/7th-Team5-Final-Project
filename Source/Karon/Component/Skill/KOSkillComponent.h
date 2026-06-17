// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Type/KOSkillTypes.h"
#include "KOSkillComponent.generated.h"

struct FKOSkillRow;
class UKOLoadSubsystem;
class UKOInventoryComponent;


// 숙제 : 로컬 플레이어 서브 시스템으로 변경 
UCLASS(ClassGroup= "KO|Skill", meta=(BlueprintSpawnableComponent))
class KARON_API UKOSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKOSkillComponent();
	
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	bool TryUnlockSkill(FName SkillName);
	bool IsUnlocked(FName SkillName) const;
	ESkillState GetSkillState(FName SkillName) const;
	FGameplayTagContainer GetUnlockedSkillTags() const;
	void GetAllSkillNames(TArray<FName>& Out) const;

private:
	UPROPERTY()
	TObjectPtr<UKOLoadSubsystem> CachedLoadSubsystem;
	UPROPERTY()
	TObjectPtr<UKOInventoryComponent> CachedInventoryComponent;
	
	TMap<FName, ESkillState> SkillStates;
	
	void InitializeSkillStates();
	void ReevaluateAllSkillStates();
	bool ArePrerequisitesMet(const FKOSkillRow& Row) const;
	const ESkillState* GetSkillInfo(FName SkillId) const;	
};
