// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOTitleWidget.generated.h"

class UCommonButtonBase;
class UKOUISubsystem;
class UCommonActivatableWidget;
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
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> LoadButton;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Widget Tag")
	FGameplayTag ConfirmationPopupTag;
	
	UPROPERTY(EditDefaultsOnly, Category = "Loading Widget")
	TSubclassOf<UCommonActivatableWidget> LoadingWidget;

	UFUNCTION(BlueprintImplementableEvent, Category = "Sound | StartButtonSound")
	void PlayStartOrLoadButtonSound() const;
private:
	void OnStartGameClicked();
	void OnQuitGameClicked();
	void OnOptionClicked() const;
	void OnLoadClicked() const;
	
	UFUNCTION()
	void StartGameConfirmation();

	UFUNCTION()
	void GameQuitConfirmation();

	UPROPERTY()
	TObjectPtr<UKOUISubsystem> CachedUISubsystem;
};
