// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOEnemyDropItemLogMessage.generated.h"

class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class KARON_API UKOEnemyDropItemLogMessage : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void SetLogMessage(FName ItemId, int32 Count);
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> ItemIconImage;
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ItemGetTextBlock;
};
