// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOInteractionNoticeUI.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class KARON_API UKOInteractionNoticeUI : public UKOActivatableWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void OnInteraction(FGameplayTag Channel, const FInstancedStruct& Payload);
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> InteractionTextBlock; 
};
