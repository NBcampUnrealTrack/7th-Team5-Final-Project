// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOWorldNotice.generated.h"

class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSetText);
/**
 * 
 */
UCLASS()
class KARON_API UKOWorldNotice : public UKOActivatableWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION()
	void WorldEnter(FGameplayTag Channel, const FInstancedStruct& Payload);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(BindWidget))
	TObjectPtr<UTextBlock> WorldNameTextBlock;

protected:
	UPROPERTY(EditAnywhere)
	float DisappearDelay=2.5f;
	


public:
	UPROPERTY(BlueprintAssignable)
	FOnSetText FOnSetTextBlock;
};
