#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOBossProjectileBase.generated.h"

class UKO_HitData;
class UGameplayEffect;
class UProjectileMovementComponent;
class USphereComponent;

USTRUCT(BlueprintType)
struct FKOBossDamageEffectData
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Level = 1.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float AttackCoefficient = 1.f;
};

UCLASS()
class KARON_API AKOBossProjectileBase : public AActor
{
	GENERATED_BODY()
 
public:
	AKOBossProjectileBase();
	
	void SetProjectile(AActor* InOwner, float InAttackPower);
 
protected:
	UPROPERTY(VisibleAnywhere, Category = "Projectile | Component")
	TObjectPtr<USphereComponent> CollisionComponent;
 
	UPROPERTY(VisibleAnywhere, Category = "Projectile | Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Damage")
	TArray<FKOBossDamageEffectData> DamageEffects;
	
	UPROPERTY(EditAnywhere, Instanced, Category = "Projectile | Damage")
	TObjectPtr<UKO_HitData> HitData;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Debug")
	bool bShowDebug = true;
	
	void ApplyDamageToTarget(AActor* TargetActor);
 
private:
	float AttackPower = 0.f;
};
