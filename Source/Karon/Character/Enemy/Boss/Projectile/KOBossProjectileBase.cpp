#include "Character/Enemy/Boss/Projectile/KOBossProjectileBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "AbilitySystem/Attribute/KOCombatSet.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

AKOBossProjectileBase::AKOBossProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
 
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetSphereRadius(30.f);
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAll"));
	CollisionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	CollisionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	RootComponent = CollisionComponent;
 
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->SetUpdatedComponent(CollisionComponent);
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->InitialSpeed = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
}

// ─── 수정 : DamageEffectClass 파라미터 제거 ─────────────────
// DamageEffects 배열은 BP에서 직접 설정
// AttackPower만 런타임에 주입
void AKOBossProjectileBase::SetProjectile(AActor* InOwner, float InAttackPower)
{
	SetOwner(InOwner);
	AttackPower = InAttackPower;
}

void AKOBossProjectileBase::ApplyDamageToTarget(AActor* TargetActor)
{
	if (!TargetActor) { return; }
 
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(TargetActor);
	if (!TargetASI) { return; }
 
	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (!TargetASC) { return; }
 
	IAbilitySystemInterface* OwnerASI = Cast<IAbilitySystemInterface>(GetOwner());
	if (!OwnerASI) { return; }
 
	UAbilitySystemComponent* OwnerASC = OwnerASI->GetAbilitySystemComponent();
	if (!OwnerASC) { return; }
 
	const UKOCombatSet* CombatSet = OwnerASC->GetSet<UKOCombatSet>();
	const float FinalAttackPower = (AttackPower > 0.f) ? AttackPower :
		(CombatSet ? CombatSet->GetAttackPower() : 1.f);
 
	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
	Context.AddSourceObject(GetOwner());

	// ─── 수정 : 단일 GE → 배열로 변경 ───────────────────────
	// BP에서 설정한 DamageEffects 배열 순회
	// AttackCoefficient로 공격별 배율 조정 가능
	for (const FKOBossDamageEffectData& Effect : DamageEffects)
	{
		if (!Effect.EffectClass) { continue; }

		FGameplayEffectSpecHandle Spec =
			OwnerASC->MakeOutgoingSpec(Effect.EffectClass, Effect.Level, Context);
		if (!Spec.IsValid()) { continue; }

		Spec.Data->SetSetByCallerMagnitude(
			KOGameplayTags::Data_AttackCoefficient,
			FinalAttackPower * Effect.AttackCoefficient);

		OwnerASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}
}
