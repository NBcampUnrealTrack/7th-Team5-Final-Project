// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOOverclockProgressBar.generated.h"

class UProgressBar;

UCLASS()
class KARON_API UKOOverclockProgressBar : public UKOActivatableWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	
	UFUNCTION()
	void ProgressBarChanged(FGameplayTag Channel, const FInstancedStruct& Payload);
	
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateRatio();

protected:
	UPROPERTY(BlueprintReadOnly)
	float Ratio = 0.f;

	UPROPERTY(BlueprintReadOnly)
	float Alpha = 0.f; 
};
