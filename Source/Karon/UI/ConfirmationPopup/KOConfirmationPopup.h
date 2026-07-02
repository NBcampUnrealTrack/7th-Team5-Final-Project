// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOConfirmationPopup.generated.h"

class UCommonTextBlock;
class UCommonButtonBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPopupClosedSignature);
/**
 * 레벨 전환 전 재확인 하는 팝업 창
 */
UCLASS()
class KARON_API UKOConfirmationPopup : public UKOActivatableWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FOnPopupClosedSignature OnConfirmed;
	
	virtual void NativeConstruct() override;
	
	void SetupPopup(const FText& Title, const FText& Description);
	
protected:
	void HandleConfirm();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CloseButton;
	
private:
	void OnCloseButtonClicked();
};
