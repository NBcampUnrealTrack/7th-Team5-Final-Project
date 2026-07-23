// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/Type/KOSkillTypes.h"
#include "KOSkillTreeLink.generated.h"

class UImage;
class UTexture2D;
class UKOSkillNodeWidget;

/**
 * 스킬트리에서 두 SkillNode를 잇는 연결선 위젯.
 * 지정된 TargetNode의 상태(ESkillState)에 따라 표시할 이미지를 바꾼다.
 */
UCLASS()
class KARON_API UKOSkillTreeLink : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// TargetNode의 현재 상태에 맞는 이미지로 갱신한다.
	UFUNCTION(BlueprintCallable, Category = "Skill Tree Link")
	void RefreshLink();

protected:
	// 이 링크가 상태를 추적할 대상 노드. 이 위젯을 배치한 WBP의 Details 패널에서 지정한다.
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Skill Tree Link")
	TObjectPtr<UKOSkillNodeWidget> TargetNode;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> LinkImage;

	UPROPERTY(EditDefaultsOnly, Category = "Skill Tree Link|Image")
	TObjectPtr<UTexture2D> LockedImage;

	UPROPERTY(EditDefaultsOnly, Category = "Skill Tree Link|Image")
	TObjectPtr<UTexture2D> CanUnlockImage;

	UPROPERTY(EditDefaultsOnly, Category = "Skill Tree Link|Image")
	TObjectPtr<UTexture2D> UnlockedImage;
};