#include "Character/Enemy/Boss/KOGA_BossAttackBase.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"

UKOGA_BossAttackBase::UKOGA_BossAttackBase()
{
	SetAssetTags(FGameplayTagContainer(KOGameplayTags::Ability_Attack));
	
	ActivationOwnedTags.AddTag(KOGameplayTags::State_Attacking);
	
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Attacking);
	ActivationBlockedTags.AddTag(KOGameplayTags::State_Groggy);
}

void UKOGA_BossAttackBase::OnMontageCompleted()
{
}

void UKOGA_BossAttackBase::OnMontageCancelled()
{
}
