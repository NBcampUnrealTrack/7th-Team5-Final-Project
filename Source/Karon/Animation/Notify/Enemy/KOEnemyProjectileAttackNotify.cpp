// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyProjectileAttackNotify.h"

#include "Character/Enemy/KOBaseEnemy.h"
#include "Character/Enemy/Projectile/KOEnemyProjectileActor.h"
#include "Game/KOProjectilePoolSubsystem.h"

UKOEnemyProjectileAttackNotify::UKOEnemyProjectileAttackNotify()
{
	bIsNativeBranchingPoint = true;
}

void UKOEnemyProjectileAttackNotify::BranchingPointNotify(FBranchingPointNotifyPayload& BranchingPointPayload)
{
	UAnimNotify::BranchingPointNotify(BranchingPointPayload);
	
	USkeletalMeshComponent* MeshComp = BranchingPointPayload.SkelMeshComponent;
	if (!MeshComp ||
		!MeshComp->GetOwner() ||
		!MeshComp->GetAnimInstance())
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
	FVector ProjectileLocation = Enemy->GetSocketLocation()+MeshComp->GetOwner()->GetActorForwardVector()*50.f;

	
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
			UE_LOG(LogTemp,Warning,TEXT("%s"),*ProjectileTransform.GetLocation().ToString());
			//TODO: 스킬 계수는 DeveloperSetting DT로 설정
			EnemyProjectile->SetProjectile(Enemy,Enemy->GetAttackPoint(),DamageMultiplier);
			EnemyProjectile->SetActiveAndCollision(true);
		}
	}
}

