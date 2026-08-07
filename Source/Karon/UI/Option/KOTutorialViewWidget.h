// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "UI/KOActivatableWidget.h"
#include "KOTutorialViewWidget.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class KARON_API UKOTutorialViewWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	FORCEINLINE void SetTutorial(const FName InName) {TutorialName=InName;}
	FORCEINLINE void SetTutorialName(const FText InText){TextBlock->SetText(InText);}
	
	UFUNCTION()
	void PlayTutorial();
	
protected:
	virtual void NativeOnInitialized() override;
	

protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Play;
	
private:
	UPROPERTY()
	FName TutorialName="None";
	
};
