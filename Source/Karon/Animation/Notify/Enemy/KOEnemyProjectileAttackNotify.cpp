// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyProjectileAttackNotify.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Ability/Enemy/KOEnemyGameplayAbility.h"
#include "Character/Enemy/KOBaseEnemy.h"
#include "Character/Enemy/Projectile/KOEnemyProjectileActor.h"
#include "Data/Type/KOEnemyType.h"
#include "Game/KOProjectilePoolSubsystem.h"
#include "SubSystem/KOEnemyDataSubsystem.h"

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
			
			//현재 활성화된 GA를 가져온다,
			UAbilitySystemComponent* AbilitySystemComponent = Enemy->GetAbilitySystemComponent();
			if (!AbilitySystemComponent)
			{
				return;
			}
			UKOEnemyGameplayAbility* EnemyGA = Cast<UKOEnemyGameplayAbility>(
				AbilitySystemComponent->GetAnimatingAbility());
			if (!EnemyGA)
			{
				return;
			}
			//GA에서 AssetTag, Enemy에서 EnemyNameTag를 가져와 세팅한다.
			FEnemySkillInfo SkillInfoTag;
			
			SkillInfoTag.SkillTag=EnemyGA->GetAssetTags().First();
			SkillInfoTag.EnemyNameTag=Enemy->EnemyNameTag;
			
			//SkillSubsystem에서 Multiplier를 찾는다.
			UKOEnemyDataSubsystem* SkillSubsystem=UKOEnemyDataSubsystem::Get(Enemy);
			if (SkillSubsystem!=nullptr)
			{
				DamageMultiplier=SkillSubsystem->GetSkillData(SkillInfoTag);
			}
			
			UE_LOG(LogTemp,Warning,TEXT("%f"),DamageMultiplier);
			//노티파이 순간의 Enemy의 AttackPoint, DamageMultiplier를 세팅한다. 
			EnemyProjectile->SetProjectile(Enemy,Enemy->GetAttackPoint(),DamageMultiplier);
			EnemyProjectile->SetActiveAndCollision(true);
		}
	}
}

