#include "KOGameplayTags_State.h"

namespace KOGameplayTags
{
	// ───────────────────────── State ──────────────────────────────
	// ── State | Character | Movement ──────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Movement_InAir, "State.Character.Movement.InAir");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Movement_Sprinting, "State.Character.Movement.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Movement_Rolling, "State.Character.Movement.Rolling");
	
	// ── State | Character | Attacking ─────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Attacking, "State.Character.Attacking");
	
	// ── State | Character | Etc ───────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Dead, "State.Character.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_StaminaExhausted, "State.Character.StaminaExhausted"); 
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Stunned, "State.Character.Stunned");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Invincible, "State.Character.Invincible");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_NoStaminaRegen, "State.Character.NoStaminaRegen");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_LockOn, "State.Character.LockOn");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_OnPlatform, "State.Character.OnPlatform");
	
	
	UE_DEFINE_GAMEPLAY_TAG(State_Character_WeaponDrawn, "State.Character.WeaponDrawn");

	// ─── Enemy Attack ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Attack_Normal, "State.Enemy.Attack.Normal");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Attack_FirstSkill, "State.Enemy.Attack.FirstSkill");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Hit_Normal, "State.Enemy.Hit.Normal");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Dead, "State.Enemy.Dead");
	
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Attacking, "State.Boss.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Dead, "State.Boss.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Groggy, "State.Boss.Groggy");
}
