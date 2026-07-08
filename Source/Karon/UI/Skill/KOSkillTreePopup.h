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
class UTextBlock;
class UKOToastMessageWidget;

/**
 * 팝업시킬 스킬창 모체
 * 
 */
UCLASS()
class KARON_API UKOSkillTreePopup : public UKOActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	void RefreshAllSkillNodes() const;
	void SetupAndBindSkillNodes();
	void RefreshActiveTooltip(UKOSkillNodeWidget* Node);
	void HandleSkillNodeClicked(UKOSkillNodeWidget* ClickedNode);
	void ShowSkillTooltip(UKOSkillNodeWidget* Node);

	UFUNCTION()
	void HandleTooltipConfirmed();

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

	// 팝업이 처음 생성될 때 SkillTooltipWidget을 갱신할 기준 노드의 인덱스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill Tree")
	int32 InitialSkillNodeIndex = 0;

	UPROPERTY()
	TObjectPtr<UKOSkillNodeWidget> ActiveTooltipNode;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UKOToastMessageWidget> ToastMessageWidget;

private:
	UPROPERTY()
	TObjectPtr<UKOLoadSubsystem> CachedLoadSubsystem;
};
