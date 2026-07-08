#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOBossCH01Platform.generated.h"

class UGameplayEffect;
class UBoxComponent;

UCLASS()
class KARON_API AKOBossCH01Platform : public AActor
{
	GENERATED_BODY()
	
public:
	AKOBossCH01Platform();
 
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
 
protected:
	UPROPERTY(VisibleAnywhere, Category = "Platform | Component")
	TObjectPtr<UStaticMeshComponent> PlatformMesh;
 
	// 낙하 중 피격 판정용
	UPROPERTY(VisibleAnywhere, Category = "Platform | Component")
	TObjectPtr<UBoxComponent> DamageCollision;
 
	UPROPERTY(VisibleAnywhere, Category = "Platform | Indicator")
	TObjectPtr<UStaticMeshComponent> IndicatorMesh;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Fall")
	float FallSpeed = 500.f;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Fall")
	float HoverDuration = 1.5f;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Option")
	float PlatformLifeSpan = 5.f;
 
	// 발판 위 판정 오차 범위
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Option")
	float PlatformTopTolerance = 15.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Debug")
	bool bShowDebug = true;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform | Damage")
	TSubclassOf<UGameplayEffect> FallDamageEffectClass;
 
private:
	bool bLanded = false;
	bool bFalling = false;
	
	UPROPERTY()
	TObjectPtr<AActor> PlayerOnPlatform;
 
	FTimerHandle LifeSpanTimerHandle;
	FTimerHandle HoverTimerHandle;
	
	void CheckPlayerOnPlatform();
	
	void SpawnIndicatorOnGround();
 
	UFUNCTION()
	void OnDamageCollisionHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);
 
	void SetOnPlatformTag(AActor* TargetActor, bool bAdd);
	void ApplyDamageToTarget(AActor* TargetActor);
	void StartFall();
	void OnLanded();
	void LifeTimeEnd();
};
