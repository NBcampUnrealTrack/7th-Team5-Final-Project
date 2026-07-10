#include "Character/Enemy/Boss/Projectile/KOBossArcProjectile.h"

#include "AbilitySystemInterface.h"
#include "KOBossShockwaveField.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AKOBossArcProjectile::AKOBossArcProjectile()
{
	ProjectileMovement->ProjectileGravityScale = 1.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}
 
void AKOBossArcProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (AActor* OwnerActor = GetOwner())
	{
		CollisionComponent->MoveIgnoreActors.Add(OwnerActor);
	}
	CollisionComponent->OnComponentHit.AddDynamic(this, &AKOBossArcProjectile::OnHit);
 
	GetWorldTimerManager().SetTimer(
		LifeSpanTimerHandle,
		this,
		&AKOBossArcProjectile::OnLifeSpanEnd,
		LifeSpan,
		false
	);
}
 
void AKOBossArcProjectile::Launch(const FVector& InVelocity)
{
	ProjectileMovement->Activate(true);
	ProjectileMovement->Velocity = InVelocity;
}
 
void AKOBossArcProjectile::OnHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	if (!OtherActor || OtherActor == GetOwner()) return;
	
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(OtherActor);
	if (TargetASI && TargetASI->GetAbilitySystemComponent())
	{
		ApplyDamageToTarget(OtherActor);
		Destroy();
		return;
	}
	
	SpawnShockwave();
	Destroy();
}
 
void AKOBossArcProjectile::SpawnShockwave()
{
	if (!ShockwaveClass)
	{
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
 
	AKOBossShockwaveField* Shockwave =
		GetWorld()->SpawnActor<AKOBossShockwaveField>(
			ShockwaveClass,
			GetActorLocation(),
			FRotator::ZeroRotator,
			SpawnParams
		);
 
	if (Shockwave)
	{
		Shockwave->SetProjectile(GetOwner(), 0.f);
	}
}
 
void AKOBossArcProjectile::OnLifeSpanEnd()
{
	Destroy();
}
