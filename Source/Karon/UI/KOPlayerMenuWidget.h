#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOPlayerMenuWidget.generated.h"

class UButton;
class UWidgetSwitcher;
class UCommonActivatableWidget;
class UKOToastMessageWidget;
class UCommonButtonBase;
class UCommonButtonGroupBase;

UENUM(BlueprintType)
enum class EKOPlayerMenuTab : uint8
{
	Inventory,
	FactoryCraft,
	SkillTree,
	Option
};

UCLASS()
class KARON_API UKOPlayerMenuWidget : public UKOActivatableWidget
{
	GENERATED_BODY()

public:
	UKOPlayerMenuWidget();
	
	UFUNCTION(BlueprintCallable)
	void SetActiveTab(EKOPlayerMenuTab Tab);
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual bool NativeOnHandleBackAction() override;
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Player Menu")
	TObjectPtr<UWidgetSwitcher> ContentSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Inventory;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Factory;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Skill;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Option;
	
	UPROPERTY()
	TObjectPtr<UCommonButtonGroupBase> TabButtonGroup;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UKOToastMessageWidget> ToastMessageWidget;

	/** Option(System) 탭 하위에서 실제 설정 팝업(KOOptionWidget)을 여는 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_OpenOptionWidget;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Save;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> Button_Load;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_BackToTitle;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Resume;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_QuitGame;
	
	UPROPERTY(EditDefaultsOnly, Category = "Loading Widget")
	TSubclassOf<UCommonActivatableWidget> DefaultLoadingWidget;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Sound | BackToTitleButtonSound")
	void PlayBackToTitleButtonSound() const;
	
private:
	UFUNCTION()
	void HandleInventoryClicked();

	UFUNCTION()
	void HandleFactoryClicked();

	UFUNCTION()
	void HandleSkillClicked();

	UFUNCTION()
	void HandleOptionClicked();

	UFUNCTION()
	void HandleOpenOptionWidgetClicked();
	
	UFUNCTION()
	void HandleSaveClicked();

	UFUNCTION()
	void HandleLoadClicked();
	
	UFUNCTION()
	void HandleBackToTitleClicked();

	UFUNCTION()
	void HandleBackToTitleConfirmed();

	UFUNCTION()
	void HandleResumeClicked();
	
	UFUNCTION()
	void HandleQuitGameClicked();
	
	UFUNCTION()
	void HandleQuitGameConfirmed();
	
	UFUNCTION()
	void HandleToastFinished();

	void ShowLocalMessage(const FText& Message, bool bCloseAfterMessage = false);

	bool bCloseMenuAfterToast = false;

private:
	bool bPausedGameByThisWidget = false;
};