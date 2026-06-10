// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Type/KOSkillTypes.h"
#include "KOSkillComponent.generated.h"

struct FKOSkillRow;
DECLARE_MULTICAST_DELEGATE(FOnSkillStateChanged);

UCLASS(ClassGroup= "KO|Skill", meta=(BlueprintSpawnableComponent))
class KARON_API UKOSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKOSkillComponent();
	
	virtual void BeginPlay() override;
	
	bool TryUnlockSkill(FName SkillName);
	bool IsUnlocked(FName SkillName) const;
	ESkillState GetSkillState(FName SkillName) const;
	FGameplayTagContainer GetUnlockedSkillTags() const;
	void GetAllSkillNames(TArray<FName>& Out) const;
	FOnSkillStateChanged OnSkillStateChanged;

private:
	UPROPERTY()
	TMap<FName, ESkillState> SkillStates;
	
	void InitializeSkillStates();
	void ReevaluateAllSkillStates();
	bool ArePrerequisitesMet(const FKOSkillRow& Row) const;
	const ESkillState* GetSkillInfo(FName SkillId) const;	
};
