#pragma once

#include "CoreMinimal.h"
#include "Character/Enemy/Boss/Projectile/KOBossProjectileBase.h"
#include "KOBossArcProjectile.generated.h"

class AKOBossShockwaveField;

UCLASS()
class KARON_API AKOBossArcProjectile : public AKOBossProjectileBase
{
	GENERATED_BODY()
 
public:
	AKOBossArcProjectile();
 
	virtual void BeginPlay() override;

	void Launch(const FVector& InVelocity);
 
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Shockwave")
	TSubclassOf<AKOBossShockwaveField> ShockwaveClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Life")
	float LifeSpan = 6.f;
 
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
	
	void SpawnShockwave();
 
	void OnLifeSpanEnd();
};
