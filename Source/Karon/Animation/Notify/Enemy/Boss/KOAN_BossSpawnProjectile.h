#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "KOAN_BossSpawnProjectile.generated.h"

class AKOBossProjectileBase;

UCLASS()
class KARON_API UKOAN_BossSpawnProjectile : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	virtual void Notify(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
 
protected:
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AKOBossProjectileBase> ProjectileClass;
	
	UPROPERTY(EditAnywhere, Category = "Projectile")
	FName SpawnSocketName = NAME_None;
	
	// 바닥 높이에 맞춰 스폰 여부
	UPROPERTY(EditAnywhere, Category = "Projectile")
	bool bSnapToGround = false;
	
	// 바닥 트레이스 최대 거리
	UPROPERTY(EditAnywhere, Category = "Projectile", meta = (EditCondition = "bSnapToGround"))
	float GroundTraceDistance = 1000.f;
};
