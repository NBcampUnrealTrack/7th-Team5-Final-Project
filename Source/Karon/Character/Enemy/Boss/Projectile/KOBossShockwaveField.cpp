#include "Character/Enemy/Boss/Projectile/KOBossShockwaveField.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "DrawDebugHelpers.h"
#include "NiagaraFunctionLibrary.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Karon/AbilitySystem/Tag/KOGameplayTags.h"
 
AKOBossShockwaveField::AKOBossShockwaveField()
{
}
 
void AKOBossShockwaveField::BeginPlay()
{
	Super::BeginPlay();
 
	if (bShowDebug)
	{
		DrawDebugCircle(
			GetWorld(),
			GetActorLocation(),
			ShockwaveRadius,
			32,
			FColor::Orange,
			false,
			1.f,
			0,
			2.f,
			FVector(1, 0, 0),
			FVector(0, 1, 0)
		);
	}
	
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(ShockwaveRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(GetOwner());
 
	GetWorld()->OverlapMultiByChannel(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		QueryParams
	);
 
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor)
		{
			continue;
		}
 
		IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(HitActor);
		if (ASI)
		{
			UAbilitySystemComponent* ASC = ASI->GetAbilitySystemComponent();
			if (ASC && ASC->HasMatchingGameplayTag(KOGameplayTags::State_Character_OnPlatform))
			{
				continue;
			}
		}
 
		if (bCanDodgeByJump)
		{
			ACharacter* HitCharacter = Cast<ACharacter>(HitActor);
			if (HitCharacter &&
				!HitCharacter->GetCharacterMovement()->IsMovingOnGround())
			{
				continue;
			}
		}
 
		ApplyDamageToTarget(HitActor);
	}

	if (ShockwaveVFX)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			ShockwaveVFX,
			GetActorLocation(),
			FRotator::ZeroRotator
		);
	}
	
	Destroy();
}
