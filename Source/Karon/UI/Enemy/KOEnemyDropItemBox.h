// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOEnemyDropItemBox.generated.h"

class UVerticalBox;
class UKOEnemyDropItemLogMessage;
/**
 * 
 */
UCLASS()
class KARON_API UKOEnemyDropItemBox : public UKOActivatableWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void ItemGet(FGameplayTag Channel, const FInstancedStruct& Payload);
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UKOEnemyDropItemLogMessage> LogItemMessageWidget;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UVerticalBox> LogItemVerticalBox;
};
