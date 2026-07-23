// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "KOEnemyBaseUI.generated.h"

/**
 * 
 */
UCLASS()
class KARON_API UKOEnemyBaseUI : public UCommonUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION( BlueprintCallable )
	void OnVisibilityChanged(bool bIsVisible);
	
	protected:
	virtual void NativeConstruct();
};
