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
 
void AKOBossProjectileBase::SetProjectile(
	AActor* InOwner,
	TSubclassOf<UGameplayEffect> InDamageEffectClass,
	float InAttackPower)
{
	SetOwner(InOwner);
	DamageEffectClass = InDamageEffectClass;
	AttackPower = InAttackPower;
}

void AKOBossProjectileBase::ApplyDamageToTarget(AActor* TargetActor)
{
	if (!DamageEffectClass || !TargetActor) { return; }
 
	IAbilitySystemInterface* TargetASI = Cast<IAbilitySystemInterface>(TargetActor);
	if (!TargetASI)
	{
		return;
	}
 
	UAbilitySystemComponent* TargetASC = TargetASI->GetAbilitySystemComponent();
	if (!TargetASC)
	{
		return;
	}
 
	IAbilitySystemInterface* OwnerASI = Cast<IAbilitySystemInterface>(GetOwner());
	if (!OwnerASI)
	{
		return;
	}
 
	UAbilitySystemComponent* OwnerASC = OwnerASI->GetAbilitySystemComponent();
	if (!OwnerASC)
	{
		return;
	}
 
	const UKOCombatSet* CombatSet = OwnerASC->GetSet<UKOCombatSet>();
	if (!CombatSet)
	{
		return;
	}
 
	const float FinalAttackPower = AttackPower > 0.f ? AttackPower : CombatSet->GetAttackPower();
 
	FGameplayEffectContextHandle Context = OwnerASC->MakeEffectContext();
	Context.AddSourceObject(GetOwner());
 
	FGameplayEffectSpecHandle Spec = OwnerASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
 
	if (Spec.IsValid())
	{
		Spec.Data->SetSetByCallerMagnitude(KOGameplayTags::Data_Damage,FinalAttackPower);
 
		OwnerASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), TargetASC);
	}
}
