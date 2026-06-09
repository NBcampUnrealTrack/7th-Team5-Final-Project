// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "KOFactoryRecipeEntryWidget.generated.h"

class UButton;
class UTextBlock;
class UKORecipeTooltipWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKOOnRecipeEntryClicked, FName, RecipeId);

UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOFactoryRecipeEntryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetRecipe(FName InRecipeId, const FText& InDisplayName);

    FName GetRecipeId() const { return RecipeId; }

    /** 엔트리 클릭 시 RecipeId와 함께 브로드캐스트. NAME_None이면 '자동 선택'. */
    UPROPERTY(BlueprintAssignable, Category = "KO|UI|Factory")
    FKOOnRecipeEntryClicked OnRecipeClicked;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UButton> RecipeButton;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> NameText;
    
    UPROPERTY(EditDefaultsOnly, Category = "KO|UI|Tooltip")
    TSubclassOf<UKORecipeTooltipWidget> TooltipClass;

private:
    UFUNCTION()
    void HandleButtonClicked();

    FName RecipeId = NAME_None;
};
