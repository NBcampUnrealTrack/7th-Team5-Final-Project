#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Boss/Projectile/KOBossProjectileBase.h"
#include "KOBossProjectile.generated.h"

UCLASS()
class KARON_API AKOBossProjectile : public AKOBossProjectileBase
{
	GENERATED_BODY()
	
public:
	AKOBossProjectile();
 
	virtual void BeginPlay() override;
 
protected:
	// 투사체 속도
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Move")
	float ProjectileSpeed = 1000.f;
 
	// 투사체 수명
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Life")
	float ProjectileLifeSpan = 5.f;
 
private:
	FTimerHandle LifeSpanTimerHandle;
 
	UFUNCTION()
	void OnHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);
 
	void LifeTimeEnd();
};
