#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KOBossProjectileBase.generated.h"

class UGameplayEffect;
class UProjectileMovementComponent;
class USphereComponent;

// ─── 추가 : 프로젝타일 전용 데미지 GE 데이터 ────────────────
// KOGA_AttackBase의 FKODamageEffectData와 동일한 구조
// Actor→GA 의존성을 피하기 위해 별도 정의
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
 
	// ─── 수정 : DamageEffectClass, AttackPower 제거
	//           DamageEffects 배열 방식으로 변경
	void SetProjectile(AActor* InOwner, float InAttackPower);
 
protected:
	UPROPERTY(VisibleAnywhere, Category = "Projectile | Component")
	TObjectPtr<USphereComponent> CollisionComponent;
 
	UPROPERTY(VisibleAnywhere, Category = "Projectile | Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// ─── 수정 : 단일 GE → 배열로 변경 ───────────────────────
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Damage")
	TArray<FKOBossDamageEffectData> DamageEffects;
 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile | Debug")
	bool bShowDebug = true;
	
	void ApplyDamageToTarget(AActor* TargetActor);
 
private:
	float AttackPower = 0.f;
};
