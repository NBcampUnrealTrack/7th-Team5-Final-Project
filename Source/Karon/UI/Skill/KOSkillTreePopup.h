// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOSkillTreePopup.generated.h"

class UKOSkillNodeWidget;
class UKOSkillSubsystem;
class UKOSkillTooltipWidget;
class UKOLoadSubsystem;
class UScrollBox;

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
	
	void RefreshAllSkillNodes() const;
	void SetupAndBindSkillNodes();
	void RefreshActiveTooltip(UKOSkillNodeWidget* Node);
	void HandleSkillNodeClicked(UKOSkillNodeWidget* ClickedNode);
	void ShowSkillTooltip(UKOSkillNodeWidget* Node);
	void HandleSkillNodeUnhovered(UKOSkillNodeWidget* Node);
	void HideSkillTooltip();
	
	UFUNCTION(BlueprintPure, Category="Skill Tree")
	UKOSkillSubsystem* GetSkillSubsystem() const { return SkillSubsystem; }

	UPROPERTY(BlueprintReadOnly, Category="Skill Tree")
	TObjectPtr<UKOSkillSubsystem> SkillSubsystem;
	
	UFUNCTION(BlueprintImplementableEvent, Category= "Skill Tree")
	TArray<UKOSkillNodeWidget*> BP_GetAllSkillNodes() const;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UKOSkillTooltipWidget> SkillTooltipWidget;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox;

	UPROPERTY()
	TArray<UKOSkillNodeWidget*> CachedSkillNodes;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly , Category="Skill Tree")
	float TooltipInterval = 20.f;
	
private:
	UPROPERTY()
	TObjectPtr<UKOLoadSubsystem> CachedLoadSubsystem;
};
