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
	
	void SetupEntry(FName InFactoryId, const FText& InDisplayName, UTexture2D* InIcon,  bool bInCanCraft);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> EntryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> FactoryIconImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FactoryNameText;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|FactoryCraft|Style")
	FLinearColor CraftableColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.3f);
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|FactoryCraft|Style")
	FLinearColor NotCraftableColor = FLinearColor(0.3f, 0.01f, 0.01f, 0.3f);

private:
	UPROPERTY()
	FName FactoryId = NAME_None;

	UFUNCTION()
	void HandleClicked();
};