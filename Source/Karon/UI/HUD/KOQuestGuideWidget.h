#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KOQuestGuideWidget.generated.h"

class UTextBlock;
class UDataTable;
class UKOQuestGuideSubsystem;

UCLASS()
class KARON_API UKOQuestGuideWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|QuestGuide")
	TObjectPtr<UDataTable> QuestGuideTable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|QuestGuide")
	FName StartQuestId;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ObjectiveText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DescriptionText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> ProgressText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> RewardText;

private:
	UFUNCTION()
	void HandleQuestChanged(FName NewQuestId);

	void RefreshQuestText();

	UPROPERTY()
	TObjectPtr<UKOQuestGuideSubsystem> CachedQuestGuide;
};