// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Component/Skill/KOSkillComponent.h"
#include "UI/KOActivatableWidget.h"
#include "KOSkillTreePopup.generated.h"

class UKOSkillNodeWidget;
class UKOSkillComponent;

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
	virtual void NativeDestruct() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	
	void RefreshAllSkillNodes() const;
	void SetupAndBindSkillNodes();
	void HandleSkillNodeClicked(UKOSkillNodeWidget* ClickedNode);
	
	UFUNCTION(BlueprintPure, Category="Skill Tree")
	UKOSkillComponent* GetSkillComponent() const {return SkillComponent;}
	
	UPROPERTY(BlueprintReadOnly, Category="Skill Tree")
	TObjectPtr<UKOSkillComponent> SkillComponent;
	
	UFUNCTION(BlueprintImplementableEvent, Category= "Skill Tree")
	TArray<UKOSkillNodeWidget*> BP_GetAllSkillNodes() const;
	
private:
	UPROPERTY()
	TArray<UKOSkillNodeWidget*> CachedSkillNodes;
};
