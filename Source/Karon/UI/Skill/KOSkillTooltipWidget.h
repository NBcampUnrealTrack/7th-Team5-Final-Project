// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Data/Type/KOSkillTypes.h"
#include "KOSkillTooltipWidget.generated.h"

class UCommonTextBlock;
class UImage;
class UVerticalBox;
class UKOSkillCostEntryWidget;
class UKOInventoryComponent;
class UKOLoadSubsystem;
struct FSlateBrush;
struct FKOSkillRow;
struct FKOItemRow;
/**
 * 해금 조건 / 개별 스킬 정보 표시용 툴팁 창
 */
UCLASS()
class KARON_API UKOSkillTooltipWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	void InitializeSkillTooltipWidget(const FKOSkillRow& SkillRow, ESkillState CurrentState,
	                                  const FText& ExecutionType, const TArray<FKOItemRow>& CostItemRows);
	void RefreshCostWidget(ESkillState NewCurrentState, const TArray<FKOItemRow>& CostItemRows);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> SkillName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> SkillClassification;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SkillIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> SkillExplanation;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> CostListContainer;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UKOSkillCostEntryWidget> CostWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FSlateBrush DefaultBrush;
private:
	UPROPERTY()
	TObjectPtr<UKOInventoryComponent> CachedInventoryComp;

	TArray<FName> CachedCostItemIds;
};
