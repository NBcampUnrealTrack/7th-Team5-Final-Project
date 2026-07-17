#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateTypes.h"
#include "KOFactoryCraftEntryWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UTexture2D;

UENUM(BlueprintType)
enum class EKOCraftTargetType : uint8
{
	Factory   UMETA(DisplayName = "Factory"),
	Equipment UMETA(DisplayName = "Equipment")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FKOFactoryCraftEntryClicked, EKOCraftTargetType, TargetType, FName, TargetId);

UCLASS()
class KARON_API UKOFactoryCraftEntryWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	FKOFactoryCraftEntryClicked OnClicked;
	
	void SetupEntry(
		EKOCraftTargetType InTargetType,
		FName InTargetId,
		const FText& InDisplayName,
		UTexture2D* InIcon,
		bool bInCanCraft,
		int32 InOwnedCount
	);
	
	void SetSelected(bool bInSelected);

	bool MatchesTarget(EKOCraftTargetType InTargetType, FName InTargetId) const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> EntryButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> FactoryIconImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> FactoryNameText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SelectionArrowText;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextAmount;
	
	UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UImage> NotCraftableImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|FactoryCraft|Selection")
	TObjectPtr<UTexture2D> SelectedNormalImage;

private:
	UPROPERTY()
	EKOCraftTargetType TargetType = EKOCraftTargetType::Factory;

	UPROPERTY()
	FName TargetId = NAME_None;

	UFUNCTION()
	void HandleClicked();
	
	void CacheDefaultButtonStyle();
	void RefreshVisualState();

	bool bCanCraft = false;
	bool bSelected = false;
	bool bDefaultStyleCached = false;

	FButtonStyle DefaultButtonStyle;
};