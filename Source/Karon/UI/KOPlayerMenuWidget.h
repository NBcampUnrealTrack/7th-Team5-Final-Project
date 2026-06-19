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
	
private:
	UFUNCTION()
	void HandleInventoryClicked();

	UFUNCTION()
	void HandleFactoryClicked();

	UFUNCTION()
	void HandleSkillClicked();

	UFUNCTION()
	void HandleOptionClicked();
	
private:
	bool bPausedGameByThisWidget = false;
};