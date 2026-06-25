// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOOverclockProgressBar.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class KARON_API UKOOverclockProgressBar : public UKOActivatableWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void ProgressBarChanged(FGameplayTag Channel, const FInstancedStruct& Payload);
	
	UFUNCTION()
	void OnNotChanged();
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> OverclockProgressBar;
	
	UPROPERTY()
	FTimerHandle TimerHandle;
	
	float DelayTime=5.f;
};
