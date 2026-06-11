#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KOFactoryCraftEntryWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKOFactoryCraftEntryClicked, FName, FactoryId);

UCLASS()
class KARON_API UKOFactoryCraftEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FKOFactoryCraftEntryClicked OnClicked;
	
	void SetupEntry(FName InFactoryId, const FText& InDisplayName, UTexture2D* InIcon);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> EntryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> FactoryIconImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FactoryNameText;

private:
	UPROPERTY()
	FName FactoryId = NAME_None;

	UFUNCTION()
	void HandleClicked();
};