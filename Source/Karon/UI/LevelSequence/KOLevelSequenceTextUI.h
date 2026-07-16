// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOLevelSequenceTextUI.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class KARON_API UKOLevelSequenceTextUI : public UKOActivatableWidget
{
	GENERATED_BODY()
	UKOLevelSequenceTextUI();
	
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void SetText(FGameplayTag Channel, const FInstancedStruct& Payload);
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> LSTextBlock;
};
