#include "Character/Enemy/Boss/Projectile/KOBossProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AKOBossProjectile::AKOBossProjectile()
{
}
 
void AKOBossProjectile::BeginPlay()
{
	Super::BeginPlay();
 
	// 충돌 이벤트 바인딩
	CollisionComponent->OnComponentHit.AddDynamic(
		this, &AKOBossProjectile::OnHit
	);
 
	// 발사 방향으로 이동
	ProjectileMovement->Activate(true);
	ProjectileMovement->Velocity =
		GetActorForwardVector() * ProjectileSpeed;
 
	// 수명 타이머
	GetWorldTimerManager().SetTimer(
		LifeSpanTimerHandle,
		this,
		&AKOBossProjectile::LifeTimeEnd,
		ProjectileLifeSpan,
		false
	);
}
 
void AKOBossProjectile::OnHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetOwner()) { return; }
 
	ApplyDamageToTarget(OtherActor);
	Destroy();
}
 
void AKOBossProjectile::LifeTimeEnd()
{
	Destroy();
}
