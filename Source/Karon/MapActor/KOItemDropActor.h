// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/Type/KOEnemyType.h"
#include "GameFramework/Actor.h"
#include "Utility/Interface/KOInteractableInterface.h"
#include "KOItemDropActor.generated.h"

UCLASS()
class KARON_API AKOItemDropActor : public AActor,public IKOInteractableInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AKOItemDropActor();
	
	virtual bool CanInteract(AActor* Interactor) const override;
	
	virtual void OnInteract(AActor* Interactor) override;


	virtual FText GetInteractionPrompt() const override;
	
	bool CheckCanGetItem();
	
protected:
	bool bHasItem=true;
	
	UPROPERTY(EditAnywhere, Category="ItemDrop")
	FEnemyDropItemInfo ItemInfo;
	
	UPROPERTY(EditAnywhere, Category="ItemDrop")
	float Percent=100.f;
};
