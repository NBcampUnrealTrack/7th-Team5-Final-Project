// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOSkillTreePopup.generated.h"

class UKOSkillNodeWidget;

/**
 * 팝업시킬 스킬창 모체
 * 
 */
UCLASS()
class KARON_API UKOSkillTreePopup : public UKOActivatableWidget
{
	GENERATED_BODY()

public:
	UKOSkillTreePopup();

protected:
	virtual void NativeConstruct() override;

	virtual void NativeOnDeactivated() override;

	void RefreshAllSkillNodes() const;

	UFUNCTION(BlueprintImplementableEvent, Category= "Skill Tree")
	TArray<UKOSkillNodeWidget*> BP_GetAllSkillNodes() const;
};
