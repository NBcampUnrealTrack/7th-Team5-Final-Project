// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyProjectileAttackNotify.h"

#include "Character/Enemy/KOBaseEnemy.h"
#include "Character/Enemy/Projectile/KOEnemyProjectileActor.h"
#include "Game/KOProjectilePoolSubsystem.h"

void UKOEnemyProjectileAttackNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                            const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	if (MeshComp==nullptr||MeshComp->GetWorld()==nullptr)
	{
		return;
	}
	
	AKOBaseEnemy* Enemy=Cast<AKOBaseEnemy>(MeshComp->GetOwner());
	if (Enemy==nullptr)
	{
		return;
	}
	
	if (Enemy->GetMesh()!=MeshComp)
	{
		return;
	}
	
	
	//발사체의 위치와 방향을 세팅합니다.
	FVector ProjectileLocation = Enemy->GetMesh()->GetSocketTransform(SocketName, RTS_World).GetLocation()+MeshComp->GetOwner()->GetActorForwardVector()*50.f;
	
	//기존 바라보는 방향대로 타겟
	FRotator ProjectileRotation=MeshComp->GetOwner()->GetActorRotation();
	
	FTransform ProjectileTransform;
	ProjectileTransform.SetLocation(ProjectileLocation);
	ProjectileTransform.SetRotation(ProjectileRotation.Quaternion());
	//발사체 풀에서 꺼낸다.
	if (UKOProjectilePoolSubsystem* ProjectileSubsystem=UKOProjectilePoolSubsystem::Get(Enemy))
	{
		AActor* Projectile=ProjectileSubsystem->GetProjectile();
		if (AKOEnemyProjectileActor* EnemyProjectile=Cast<AKOEnemyProjectileActor>(Projectile))
		{
			EnemyProjectile->SetActorTransform(ProjectileTransform);
			//TODO: 스킬 계수는 DeveloperSetting DT로 설정
			EnemyProjectile->SetProjectile(Enemy,Enemy->GetAttackPoint(),DamageMultiplier);
			EnemyProjectile->SetActiveAndCollision(true);
		}
	}
	
	
}
