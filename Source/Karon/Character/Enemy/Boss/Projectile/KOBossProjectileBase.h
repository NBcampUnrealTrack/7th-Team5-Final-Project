#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOBossProjectileBase.generated.h"

class UGameplayEffect;
class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class KARON_API AKOBossProjectileBase : public AActor
{
	GENERATED_BODY()
 
public:
	AKOBossProjectileBase();
 
	// 스폰 시 발사 정보 설정
	void SetProjectile(
		AActor* InOwner,
		TSubclassOf<UGameplayEffect> InDamageEffectClass,
		float InAttackPower
	);
 
protected:
	UPROPERTY(VisibleAnywhere, Category = "Projectile | Component")
	TObjectPtr<USphereComponent> CollisionComponent;
 
	UPROPERTY(VisibleAnywhere, Category = "Projectile | Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Damage")
	TSubclassOf<UGameplayEffect> DamageEffectClass;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Debug")
	bool bShowDebug = true;
	
	void ApplyDamageToTarget(AActor* TargetActor);
 
private:
	float AttackPower = 0.f;
};
