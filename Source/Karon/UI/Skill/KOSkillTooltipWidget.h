// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Data/Type/KOSkillTypes.h"
#include "KOSkillTooltipWidget.generated.h"

class UCommonTextBlock;
class UImage;
class UVerticalBox;
class UCommonButtonBase;
class UKOSkillCostEntryWidget;
class UKOInventoryComponent;
class UKOLoadSubsystem;
struct FSlateBrush;
struct FKOSkillRow;
struct FKOItemRow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTooltipConirmed);
/**
 * 해금 조건 / 개별 스킬 정보 표시용 툴팁 창
 */
UCLASS()
class KARON_API UKOSkillTooltipWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FOnTooltipConirmed OnConfirmed;

	void InitializeSkillTooltipWidget(const FKOSkillRow& SkillRow, ESkillState CurrentState,
	                                  const FText& ExecutionType, const TArray<FKOItemRow>& CostItemRows);
	void RefreshCostWidget(ESkillState NewCurrentState, const TArray<FKOItemRow>& CostItemRows);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void HandleConfirmButtonClicked();

	/** CurrentState가 CanUnlock일 때만 ConfirmButton을 누를 수 있도록 활성화/잠금 오버레이를 갱신. */
	void RefreshConfirmButtonState(ESkillState CurrentState);

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
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> ConfirmButton;

	/** CanUnlock 상태가 아닐 때(Locked/Unlocked) ConfirmButton 위를 덮어 클릭을 막는 잠금 표시 이미지. */
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> ConfirmButtonLockedOverlay;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UKOSkillCostEntryWidget> CostWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	FSlateBrush DefaultBrush;
private:
	UPROPERTY()
	TObjectPtr<UKOInventoryComponent> CachedInventoryComp;

	TArray<FName> CachedCostItemIds;
};
