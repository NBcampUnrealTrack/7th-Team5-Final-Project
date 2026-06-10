// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Data/Type/KOSkillTypes.h"
#include "KOSkillNodeWidget.generated.h"

class UImage;
class UKOSkillComponent;

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
	void InitializeNode(FName InSkillName, FGameplayTag InSkillTag, TArray<FSkillCost> InCost, ESkillState InState);
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
protected:
	// CommonButtonBase의 클릭 이벤트를 오버라이드
	virtual void NativeOnClicked() override;

	void RefreshNode();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> OverlayImage;
	// 상태에 따른 색상 변경용 노출 변수
	UPROPERTY(EditDefaultsOnly, Category= "SKill|UI Color")
	FLinearColor LockedColor    = FLinearColor(0.2f, 0.2f, 0.2f, 0.8f);
	UPROPERTY(EditDefaultsOnly, Category= "SKill|UI Color")
	FLinearColor CanUnlockColor = FLinearColor(1.f, 1.f, 0.5f, 0.4f);
	UPROPERTY(EditDefaultsOnly, Category= "SKill|UI Color")
	FLinearColor UnlockedColor  = FLinearColor(1.f, 1.f, 1.f, 0.1f);
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FName SkillName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Skill")
	FGameplayTag SkillTag;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	TArray<FSkillCost> SkillCosts;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	ESkillState CurrentState;
	
private:
	TWeakObjectPtr<UKOSkillComponent> CachedSkillComponent;
};
