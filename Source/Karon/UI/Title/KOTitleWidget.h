#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOTitleWidget.generated.h"

class UCommonButtonBase;
class UKOUISubsystem;
struct FGameplayTag;
/**
 * 타이틀 화면상의 버튼 총괄 위젯
 */
UCLASS()
class KARON_API UKOTitleWidget : public UKOActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> StartGameButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> QuitGameButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> OptionButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget Tag")
	FGameplayTag ConfirmationPopupTag;

private:
	void OnStartGameClicked() const;
	void OnQuitGameClicked();
	void OnOptionClicked() const;

	UFUNCTION()
	void GameQuitConfirmation();

	UPROPERTY()
	TObjectPtr<UKOUISubsystem> CachedUISubsystem;
};
