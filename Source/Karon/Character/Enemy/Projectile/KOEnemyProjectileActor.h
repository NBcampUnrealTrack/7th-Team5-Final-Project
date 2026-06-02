// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOEnemyProjectileActor.generated.h"
class UGameplayEffect;
class UProjectileMovementComponent;
class USphereComponent;
class AKOBaseEnemy;
/**
 * 소환한 대상의 정면 방향으로 날아가며 Sphere Trace를 진행해, 맞으면 데미지를 입힙니다.
 */
UCLASS()
class KARON_API AKOEnemyProjectileActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AKOEnemyProjectileActor();
	void SetActiveAndCollision(bool InActive);
	void SetProjectile(AKOBaseEnemy* InEnemy,float AttackPoint,float DamageMultiplier);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp, 
		FVector NormalImpulse, 
		const FHitResult& Hit
		);
	void LifeTimeEnd();
	void ReturnToPool();

private:
	UPROPERTY(VisibleAnywhere)
	float ProjectileSpeed=1000.0f;	
	
	UPROPERTY(VisibleAnywhere)
	bool bIsHoming=false;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComponent;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> ProjectileStaticMesh;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent; 
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<AKOBaseEnemy> Enemy;
	
	UPROPERTY()
	FTimerHandle TimerHandle;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UGameplayEffect> DamageEffectClass;
	
	float ProjectileDamage=0.0f;	
};
