// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "KOProjectilePoolSubsystem.generated.h"

/**
* WorldSubsystem으로, 발사체의 오브젝트 풀을 관리합니다.
 */
UCLASS()
class KARON_API UKOProjectilePoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public: 
	static UKOProjectilePoolSubsystem* Get(UObject* WorldContext);
	virtual void Initialize(FSubsystemCollectionBase&);
	TObjectPtr<AActor> GetProjectile();
	void ReturnToPool(AActor* Projectile);
	
private:
	UPROPERTY()
	TArray<TObjectPtr<AActor>> ProjectilePool;
	
	FTransform ProjectileTransform;
	float PoolSize=100;
	//발생할수도 있는 시점의 충돌 문제 방지위한 초기 스폰 위치
	FVector PoolLocation=FVector(-2000.f,-2000.f,-1000.f);

};
