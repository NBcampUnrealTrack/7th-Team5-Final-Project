// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOSkillTreePopup.generated.h"

class UKOSkillNodeWidget;
class UKOSkillTreeLink;
class UKOSkillSubsystem;
class UKOSkillTooltipWidget;
class UKOLoadSubsystem;
class UKOToastMessageWidget;
class UScrollBox;
class UTextBlock;
class UCommonButtonGroupBase;

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
	void RefreshAllSkillLinks() const;
	void SetupAndBindSkillNodes();
	void RefreshActiveTooltip(UKOSkillNodeWidget* Node);
	void HandleSkillNodeClicked(UKOSkillNodeWidget* ClickedNode);
	void ShowSkillTooltip(UKOSkillNodeWidget* Node);

	UFUNCTION()
	void HandleTooltipConfirmed();

	UFUNCTION(BlueprintPure, Category="Skill Tree")
	UKOSkillSubsystem* GetSkillSubsystem() const { return SkillSubsystem; }
	
	UFUNCTION(BlueprintImplementableEvent, Category= "Skill Tree")
	TArray<UKOSkillNodeWidget*> BP_GetAllSkillNodes() const;

	UFUNCTION(BlueprintImplementableEvent, Category= "Skill Tree")
	TArray<UKOSkillTreeLink*> BP_GetAllSkillLinks() const;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Sound")
	void PlayUnlockSuccessSound();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Sound")
	void PlayUnlockFailedSound();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly , Category="Skill Tree")
	float TooltipInterval = 20.f;

	// 팝업이 처음 생성될 때 SkillTooltipWidget을 갱신할 기준 노드의 인덱스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Skill Tree")
	int32 InitialSkillNodeIndex = 0;
	
	UPROPERTY(BlueprintReadOnly, Category="Skill Tree")
	TObjectPtr<UKOSkillSubsystem> SkillSubsystem;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UKOSkillTooltipWidget> SkillTooltipWidget;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UKOToastMessageWidget> ToastMessageWidget;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox;
	
	UPROPERTY()
	TObjectPtr<UCommonButtonGroupBase> SkillNodeGroup;
	
	UPROPERTY()
	TArray<UKOSkillNodeWidget*> CachedSkillNodes;

	UPROPERTY()
	TArray<UKOSkillTreeLink*> CachedSkillLinks;

	UPROPERTY()
	TObjectPtr<UKOSkillNodeWidget> ActiveTooltipNode;

private:
	UPROPERTY()
	TObjectPtr<UKOLoadSubsystem> CachedLoadSubsystem;
};
