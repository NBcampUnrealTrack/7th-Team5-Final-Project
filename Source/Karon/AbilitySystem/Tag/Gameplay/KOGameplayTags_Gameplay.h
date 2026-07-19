#pragma once

#include "NativeGameplayTags.h"

namespace KOGameplayTags
{
	// ───────────────────────── Ability ─────────────────────────────
	
	
	// ───────────────────────── Effect ─────────────────────────────
	// ── Effect | Damage ───────────────────────────────────────────
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Damage);
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Heal);

	// ── Effect | Buff ─────────────────────────────────────────────
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Buff_MoveSpeed);
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Buff_AttackPower);

	// ── Effect | Debuff ───────────────────────────────────────────
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Debuff_Slow);
	// UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Debuff_Stun);
	
	
	// ───────────────────────── Cue ─────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_HitImpact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Death);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_SuperArmor);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_OverClock_Activate);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_OverClock_Aura);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_CameraShake_Attacker);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_CameraShake_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_AirPound);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_CounterAttack);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_WeaponTrail_Sword);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_WeaponTrail_Hammer);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Hammer_Hit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(GameplayCue_Hammer_Overlay);
	
}
