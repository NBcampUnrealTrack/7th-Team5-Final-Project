#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOPlayerMenuWidget.generated.h"

class UButton;
class UWidgetSwitcher;

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
	TObjectPtr<UButton> Button_Inventory;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Factory;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Skill;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Option;

	/** Option(System) 탭 하위에서 실제 설정 팝업(KOOptionWidget)을 여는 버튼 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_OpenOptionWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_BackToTitle;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Resume;
	
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
	void HandleBackToTitleClicked();

	UFUNCTION()
	void HandleBackToTitleConfirmed();

	UFUNCTION()
	void HandleResumeClicked();

private:
	bool bPausedGameByThisWidget = false;
};