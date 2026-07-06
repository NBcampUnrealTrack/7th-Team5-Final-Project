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
	
	FName GetClusterSaveId() const { return ClusterSaveId; }
	int32 GetSpawnedEnemiesCountForLoad() const { return SpawnedEnemiesCount; }

	// 세이브 로드
	void ResetClusterForLoad();
	void RegisterSpawnedEnemyForLoad(AKOBaseEnemy* Enemy);
	void ScheduleRespawnForLoad();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void SpawnEnemies();
	
	UFUNCTION()
	void OnDestroyedEnemy();

protected:
	UPROPERTY(EditAnywhere,Category="Enemy|Map")
	TMap<TSubclassOf<AKOBaseEnemy>,int32> EnemyMap;
	
	//클러스터의 레벨(에너미들의 레벨)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
	int32 Level = 1;
	
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
	
	//에너미 전부 처치된 이후 재스폰 인터벌
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Settings")
	int32 SpawnInterval = 15.f;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, Category = "KO|Save")
	FName ClusterSaveId = NAME_None;

private:
	float ProjectionDistance=200.f;
	float EnemyZOffset=90.f;
	
	FTimerHandle SpawnTimerHandle;
	
	int32 SpawnedEnemiesCount=0;
	int32 DestroyedEnemyCnt=0;
	
	int32 SpawnWaveIndex = 0;
};
