// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Type/KOSkillTypes.h"
#include "KOSkillComponent.generated.h"


struct FKOSkillRow;

UCLASS(ClassGroup= "KO|Skill", meta=(BlueprintSpawnableComponent))
class KARON_API UKOSkillComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKOSkillComponent();
	
	bool TryUnlockSkill(FName SkillId);
	ESkillState GetSkillState(FName SkillId) const;
	bool IsUnlocked(FName SkillId) const;
	FGameplayTagContainer GetUnlockedSkillTags() const;
	void GetAllSkillIds(TArray<FName>& Out) const;
	

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TMap<FName, ESkillState> SkillStates;
	
	void InitializeSkillStates();
	void ReevaluateAllSkillStates();
	bool ArePrerequisitesMet(const FKOSkillRow& Row) const;
	const ESkillState* GetSkillInfo(FName SkillId) const;	
};
