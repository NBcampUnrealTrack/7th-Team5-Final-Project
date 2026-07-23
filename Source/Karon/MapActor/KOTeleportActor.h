// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Utility/Interface/KOInteractableInterface.h"
#include "KOTeleportActor.generated.h"

UCLASS()
class KARON_API AKOTeleportActor : public AActor,public IKOInteractableInterface
{
	GENERATED_BODY()

public:
	AKOTeleportActor();
	
	virtual bool CanInteract(AActor* Interactor) const override;
	
	virtual void OnInteract(AActor* Interactor) override;
	
	virtual FText GetInteractionPrompt() const override;

protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<AActor> TargetTeleportActor;
	
};
