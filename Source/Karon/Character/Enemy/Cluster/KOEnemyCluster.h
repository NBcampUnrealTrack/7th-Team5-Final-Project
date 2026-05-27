// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "GameFramework/Actor.h"
#include "KOEnemyCluster.generated.h"

class AKOBaseEnemy;

UCLASS()
class KARON_API AKOEnemyCluster : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AKOEnemyCluster();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	
private:
	void SpawnEnemies();
	
protected:
	UPROPERTY(EditAnywhere,Category="Enemy|Map")
	TMap<TSubclassOf<AKOBaseEnemy>,int32> EnemyMap;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UBoxComponent> SpawningBox;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Spawning")
	USceneComponent* Scene;
	
	//푸아송 디스크의 표본 사이 최소 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	float MinDistanceBetweenEnemies = 300.f;
	
	//푸아송 디스크의 표본 추출 시도 최대 횟수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	int32 MaxAttemptsPerPoint = 30;
	
private:
	float ProjectionDistance=200.f;
	float EnemyZOffset=90.f;
};
