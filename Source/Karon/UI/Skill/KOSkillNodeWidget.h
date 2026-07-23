// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Data/Type/KOSkillTypes.h"
#include "KOSkillNodeWidget.generated.h"

class UImage;
class UTextBlock;
class UCommonButtonBase;
class UKOSkillSubsystem;
class UTexture2D;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnSkillNodeClicked, UKOSkillNodeWidget*)

/**
 * 스킬창에서 스킬 한 칸을 담당하는 클래스
 */
UCLASS()
class KARON_API UKOSkillNodeWidget : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	FOnSkillNodeClicked OnSkillNodeClicked;
	// 외부(팝업)에서 노드를 초기화할 때 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void InitializeNode(const FName& InSkillName, FGameplayTag InSkillTag, TArray<FSkillCost> InCost,
	                   ESkillState InState, ESkillExecutionType InExType, UTexture2D* InIcon);

	// Tooltip의 ConfirmButton이 눌렸을 때 팝업에서 호출하여 실제 해금 시도를 수행하는 함수
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void ExecuteUnlock();

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	FName GetSkillName() const { return SkillName; }
	ESkillState GetCurrentState() const { return CurrentState; }

protected:
	virtual void NativeOnClicked() override;
	virtual void NativeOnSelected(bool bBroadcast) override;
	virtual void NativeOnDeselected(bool bBroadcast) override;

	virtual FReply
	NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	                                  UDragDropOperation*& OutOperation) override;

	void RefreshNode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Drag")
	FVector2D DragVisualSize = FVector2D(64.f, 64.f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skill|Drag")
	float DragVisualOpacity = 0.85f;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> SkillIcon;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> SelectedImage;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> BackGroundImage;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> CurrentTypeText;
	
	UPROPERTY(EditDefaultsOnly, Category= "SKill|Background Image")
	TObjectPtr<UTexture2D> lockedImage;
	
	UPROPERTY(EditDefaultsOnly, Category= "SKill|Background Image")
	TObjectPtr<UTexture2D> CanUnlockImage;
	
	UPROPERTY(EditDefaultsOnly, Category= "SKill|Background Image")
	TObjectPtr<UTexture2D> UnlockedImage;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName SkillName;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	ESkillState CurrentState;
	
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	ESkillExecutionType CurrentExType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FGameplayTag SkillTag;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TArray<FSkillCost> SkillCosts;

private:
	TWeakObjectPtr<UKOSkillSubsystem> CachedSkillSubsystem;

	UPROPERTY()
	FSlateColor OriginalBrushTint;
	
	bool CanDragThisSkill() const;

	/** OnSkillNodeClicked를 브로드캐스트해 Tooltip을 이 노드 기준으로 갱신시킨다. (클릭 / 드래그 시작 공용) */
	void NotifySkillNodeClicked();
	
	void SetCurrentTypeText();
};
