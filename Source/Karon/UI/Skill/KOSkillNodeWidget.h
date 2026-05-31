// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "Data/Type/KOSkillTypes.h"
#include "KOSkillNodeWidget.generated.h"

/**
 * 스킬창에서 스킬 한 칸을 담당하는 클래스
 */
UCLASS()
class KARON_API UKOSkillNodeWidget : public UCommonButtonBase
{
	GENERATED_BODY()
	
public:
	// 외부(팝업)에서 노드를 초기화할 때 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Skill")
	void InitializeNode(FGameplayTag InSkillTag, FSkillCost InCost, ESkillState InState);
	
	virtual void NativeConstruct() override;

protected:
	// CommonButtonBase의 클릭 이벤트를 오버라이드
	virtual void NativeOnClicked() override;

	// 상태가 변경되었을 때 블루프린트에서 연출(애니메이션, 색상 변경)을 할 수 있도록 이벤트 제공
	UFUNCTION(BlueprintImplementableEvent, Category = "Skill")
	void BP_OnSkillStateChanged(ESkillState NewState);

public:
	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FGameplayTag SkillTag;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	FSkillCost SkillCost;

	UPROPERTY(BlueprintReadOnly, Category = "Skill")
	ESkillState CurrentState;
};
