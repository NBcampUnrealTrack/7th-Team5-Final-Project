#pragma once

#include "CoreMinimal.h"
#include "KOActivatableWidget.h"
#include "KOTitleWidget.generated.h"

class UCommonButtonBase;

/**
 * 타이틀 화면상의 버튼 총괄 위젯
 */
UCLASS()
class KARON_API UKOTitleWidget : public UKOActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> StartGameButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> QuitGameButton;
	
private:
	
	void OnStartGameClicked() const;
	void OnQuitGameClicked() const;
};
