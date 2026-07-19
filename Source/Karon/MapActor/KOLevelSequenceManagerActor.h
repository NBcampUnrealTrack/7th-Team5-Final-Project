// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LevelSequence.h"
#include "GameFramework/Actor.h"
#include "KOLevelSequenceManagerActor.generated.h"


UCLASS()
class KARON_API AKOLevelSequenceManagerActor : public AActor
{
	GENERATED_BODY()

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere,Category="Data Table")
	TObjectPtr<UDataTable> LevelSequenceDataTable;
	
private:
	UPROPERTY()
	TMap<FName,ULevelSequence*> LevelSequenceDataTableMap;
};
