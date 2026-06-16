// Fill out your copyright notice in the Description page of Project Settings.


#include "KOProjectilePoolSubsystem.h"

#include "Character/Enemy/Projectile/KOEnemyProjectileActor.h"

UKOProjectilePoolSubsystem* UKOProjectilePoolSubsystem::Get(UObject* WorldContext)
{
	if (!IsValid(WorldContext)||!WorldContext->GetWorld())
	{
		return nullptr;
	}
	return WorldContext->GetWorld()->GetSubsystem<UKOProjectilePoolSubsystem>();
}

void UKOProjectilePoolSubsystem::Initialize(FSubsystemCollectionBase& SubsystemCollectionBase)
{
	Super::Initialize(SubsystemCollectionBase);
	
	if (GetWorld()==nullptr||!GetWorld()->IsGameWorld())
	{
		return;
	}
	
	ProjectileTransform.SetLocation(PoolLocation);
	
	for (int32 i=0;i<PoolSize;i++)
	{
		TObjectPtr<AActor> Projectile=GetWorld()->SpawnActor<AKOEnemyProjectileActor>();
		Projectile->SetActorTransform(ProjectileTransform);
		ProjectilePool.Add(Projectile);
	}
}

TObjectPtr<AActor> UKOProjectilePoolSubsystem::GetProjectile()
{
	for (int32 i=0;i<PoolSize;i++)
	{
		if (ProjectilePool[i]->IsHidden())
		{
			return ProjectilePool[i];
		}
	}
	return nullptr;
}

void UKOProjectilePoolSubsystem::ReturnToPool(AActor* Projectile)
{
	TObjectPtr<AKOEnemyProjectileActor> ProjectileActor = Cast<AKOEnemyProjectileActor>(Projectile);
	if (ProjectileActor == nullptr)
	{
		return;
	}
	ProjectileActor->SetActiveAndCollision(false);
	ProjectileActor->SetActorTransform(ProjectileTransform);
}
