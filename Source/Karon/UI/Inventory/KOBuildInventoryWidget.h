#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOBuildInventoryWidget.generated.h"

class UButton;
class UWidgetSwitcher;
class UKOFactoryCraftWidget;
class UCommonButtonBase;
class UCommonButtonGroupBase;

UENUM(BlueprintType)
enum class EKOBuildInventoryTab : uint8
{
	Inventory,
	FactoryCraft
};

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOBuildInventoryWidget : public UKOActivatableWidget
{
	GENERATED_BODY()

public:
	UKOBuildInventoryWidget();
	
	UFUNCTION(BlueprintCallable)
	void SetActiveTab(EKOBuildInventoryTab Tab);
	
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	//virtual bool NativeOnHandleBackAction() override;
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget), Category = "Player Menu")
	TObjectPtr<UWidgetSwitcher> ContentSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Inventory;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_Factory;
	
	UPROPERTY()
	TObjectPtr<UCommonButtonGroupBase> TabButtonGroup;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UKOFactoryCraftWidget> FactoryCraftWidget;
	
private:
	UFUNCTION()
	void HandleInventoryClicked();

	UFUNCTION()
	void HandleFactoryClicked();
};