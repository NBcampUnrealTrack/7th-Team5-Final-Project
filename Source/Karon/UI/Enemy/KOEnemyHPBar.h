// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "KOEnemyHPBar.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class KARON_API UKOEnemyHPBar : public UCommonUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION()
	void OnHPChanged(float Percent);
protected:
	virtual void NativeConstruct() override;	
	

private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UProgressBar> HPProgressBar;
};
