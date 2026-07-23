// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOGameOverWidget.generated.h"

class UCommonButtonBase;
/**
 * 
 */
UCLASS()
class KARON_API UKOGameOverWidget : public UKOActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> BackToTitleButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> BackToLastSaveButton;
	
	UPROPERTY(EditDefaultsOnly, Category = "Loading Widget")
	TSubclassOf<UCommonActivatableWidget> DefaultLoadingWidget;
	
private:
	void OnBackToTitleClicked();
	
	void OnBackToLastSaveClicked();
};
