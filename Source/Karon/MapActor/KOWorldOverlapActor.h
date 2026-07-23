// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOWorldOverlapActor.generated.h"

class UBoxComponent;

UCLASS()
class KARON_API AKOWorldOverlapActor : public AActor
{
	GENERATED_BODY()
	AKOWorldOverlapActor();
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
	


protected:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> OverlapBox;
	
	UPROPERTY(EditAnywhere,Category="WorldName")
	FText WorldName;
};
