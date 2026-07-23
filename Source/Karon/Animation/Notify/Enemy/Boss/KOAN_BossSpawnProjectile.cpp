#include "Animation/Notify/Enemy/Boss/KOAN_BossSpawnProjectile.h"

#include "Character/Enemy/Boss/Projectile/KOBossProjectileBase.h"

void UKOAN_BossSpawnProjectile::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp || !MeshComp->GetOwner())
	{
		return;
	}
 
	AActor* Owner = MeshComp->GetOwner();
 
	if (!ProjectileClass)
	{
		return;
	}
 
	// 스폰 위치 결정
	// 소켓 이름 설정 시 소켓 위치 사용
	// 미설정 시 보스 위치 사용
	FVector SpawnLocation = SpawnSocketName != NAME_None ?
		MeshComp->GetSocketLocation(SpawnSocketName) :
		Owner->GetActorLocation();
 
	// 바닥 높이 체크 
	if (bSnapToGround)
	{
		FHitResult GroundHit;
		FVector TraceStart = SpawnLocation;
		FVector TraceEnd = SpawnLocation - FVector(0.f, 0.f, GroundTraceDistance);
 
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Owner);
 
		if (Owner->GetWorld()->LineTraceSingleByChannel(
			GroundHit,
			TraceStart,
			TraceEnd,
			ECC_WorldStatic,
			QueryParams
		))
		{
			// 바닥 위치로 Z값 보정
			SpawnLocation.Z = GroundHit.ImpactPoint.Z;
		}
	}
 
	FRotator SpawnRotation = Owner->GetActorRotation();
 
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Owner;
	SpawnParams.Instigator = Cast<APawn>(Owner);

	AKOBossProjectileBase* Projectile =
		Owner->GetWorld()->SpawnActor<AKOBossProjectileBase>(
			ProjectileClass,
			SpawnLocation,
			SpawnRotation,
			SpawnParams
		);
 
	if (!Projectile)
	{
		return;
	}
	
	Projectile->SetProjectile(Owner, 0.f);
}
