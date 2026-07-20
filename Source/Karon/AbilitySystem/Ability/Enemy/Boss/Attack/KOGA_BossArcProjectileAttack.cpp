#include "AbilitySystem/Ability/Enemy/Boss/Attack/KOGA_BossArcProjectileAttack.h"

#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Character/Enemy/Boss/KOBossBase.h"
#include "Character/Enemy/Boss/Projectile/KOBossArcProjectile.h"

UKOGA_BossArcProjectileAttack::UKOGA_BossArcProjectileAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}
 
void UKOGA_BossArcProjectileAttack::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
 
	WaitLaunchEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, 
		LaunchEventTag, 
		nullptr, 
		true, 
		false
		);
	WaitLaunchEventTask->EventReceived.AddDynamic(this, &UKOGA_BossArcProjectileAttack::OnLaunchEventReceived);
	WaitLaunchEventTask->ReadyForActivation();
}
 
void UKOGA_BossArcProjectileAttack::OnLaunchEventReceived(FGameplayEventData Payload)
{
	CurrentBurst = 0;
	StartBurst();
}
 
void UKOGA_BossArcProjectileAttack::StartBurst()
{
	if (CurrentBurst >= BurstCount)
	{
		return;
	}
	
	SpawnedCount = 0;
 
	BurstStartAngle = bRandomStartAnglePerBurst ? FMath::RandRange(0.f, 360.f) : 0.f;
  
	if (SpawnInterval <= 0.f)
	{
		for (int32 i = 0; i < ProjectileCount; ++i)
		{
			SpawnOneProjectile();
		}
		CurrentBurst++;

		if (CurrentBurst < BurstCount)
		{
			GetWorld()->GetTimerManager().SetTimer(
				BurstTimerHandle,
				this,
				&UKOGA_BossArcProjectileAttack::StartBurst,
				BurstInterval,
				false
			);
		}
		else
		{
			OnAllBurstsCompleted();
		}
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(
			SpawnTimerHandle,
			this,
			&UKOGA_BossArcProjectileAttack::SpawnOneProjectile,
			SpawnInterval,
			true,
			0.f
		);
	}
}
 
void UKOGA_BossArcProjectileAttack::SpawnOneProjectile()
{
	if (SpawnedCount >= ProjectileCount)
	{
		return;
	}
	
	AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());
	if (!Boss || !ProjectileClass)
	{
		return;
	}
	
	const float AngleStep = 360.f / FMath::Max(ProjectileCount, 1);
	const float CurrentAngle = BurstStartAngle + AngleStep * SpawnedCount;
	const float AngleRad = FMath::DegreesToRadians(CurrentAngle);
 
	const FVector HorizontalDir = FVector(
		FMath::Cos(AngleRad),
		FMath::Sin(AngleRad),
		0.f
	);
 
	const float RandomLaunchSpeedXY = FMath::RandRange(MinLaunchSpeedXY, MaxLaunchSpeedXY);
	const FVector LaunchVelocity = HorizontalDir * RandomLaunchSpeedXY + FVector::UpVector * LaunchSpeedZ;
	const float SpawnOffsetRadius = 50.f;
	const FVector SpawnLocation = Boss->GetActorLocation() + FVector(0.f, 0.f, 50.f) + HorizontalDir * SpawnOffsetRadius;  // 발사 방향으로 밀어서 스폰
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Boss;
	SpawnParams.Instigator = Boss;
 
	AKOBossArcProjectile* Projectile =
		GetWorld()->SpawnActor<AKOBossArcProjectile>(
			ProjectileClass,
			SpawnLocation,
			HorizontalDir.Rotation(),
			SpawnParams
		);
 
	if (Projectile)
	{
		Projectile->SetProjectile(Boss, 0.f);
		Projectile->Launch(LaunchVelocity);
	}
	
	SpawnedCount++;
	
	if (SpawnedCount >= ProjectileCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
 
		CurrentBurst++;
		if (CurrentBurst < BurstCount)
		{
			GetWorld()->GetTimerManager().SetTimer(
				BurstTimerHandle,
				this,
				&UKOGA_BossArcProjectileAttack::StartBurst,
				BurstInterval,
				false
			);
		}
		else
		{
			OnAllBurstsCompleted();
		}
	}
}
 
void UKOGA_BossArcProjectileAttack::OnAllBurstsCompleted()
{
	GetWorld()->GetTimerManager().SetTimer(
		EndDelayTimerHandle,
		this,
		&UKOGA_BossArcProjectileAttack::StopMontageAndEnd,
		FMath::Max(EndDelay, 0.f),
		false
	);
}
 
void UKOGA_BossArcProjectileAttack::StopMontageAndEnd()
{
	ACharacter* Character = GetAvatarCharacter();
	if (!Character)
	{
		return;
	}
	
	UAnimInstance* AnimInst = Character->GetMesh()->GetAnimInstance();
	if (AnimInst && AttackMontage)
	{
		AnimInst->Montage_Stop(0.25f, AttackMontage);
	}
}
 
void UKOGA_BossArcProjectileAttack::OnMontageCompleted()
{
	ApplyCooldown(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
 
void UKOGA_BossArcProjectileAttack::OnMontageCancelled()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
 
void UKOGA_BossArcProjectileAttack::EndAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility,
	bool bWasCancelled)
{
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(BurstTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(EndDelayTimerHandle);
 
	if (WaitLaunchEventTask)
	{
		WaitLaunchEventTask->EndTask();
		WaitLaunchEventTask = nullptr;
	}
	
	AKOBossBase* Boss = Cast<AKOBossBase>(GetAvatarActorFromActorInfo());
	if (Boss)
	{
		Boss->NotifyGimmickDashEnd();
	}
	
	SpawnedCount = 0;
	CurrentBurst = 0;
 
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
