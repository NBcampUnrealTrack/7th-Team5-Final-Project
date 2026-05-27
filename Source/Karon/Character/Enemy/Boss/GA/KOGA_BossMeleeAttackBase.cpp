#include "Character/Enemy/Boss/GA/KOGA_BossMeleeAttackBase.h"

#include "GameFramework/Character.h"

UKOGA_BossMeleeAttackBase::UKOGA_BossMeleeAttackBase()
{
}
 
FVector UKOGA_BossMeleeAttackBase::GetAttackSocketLocation() const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		return FVector::ZeroVector;
	}
 
	ACharacter* Character = Cast<ACharacter>(Avatar);
	if (!Character)
	{
		return Avatar->GetActorLocation();
	}
 
	USkeletalMeshComponent* Mesh = Character->GetMesh();
	if (!Mesh)
	{
		return Avatar->GetActorLocation();
	}
 
	return Mesh->GetSocketLocation(AttackSocketName);
}
