#include "KOGameplayTags_State.h"

namespace KOGameplayTags
{
	// ───────────────────────── State ──────────────────────────────
	// ── State | Character | Movement ──────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Movement_InAir, "State.Character.Movement.InAir");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Movement_Sprinting, "State.Character.Movement.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Movement_Rolling, "State.Character.Movement.Rolling");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Movement_Dodging, "State.Character.Movement.Dodging");
	
	// ── State | Character | Attacking ─────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Attacking, "State.Character.Attacking");
	
	// ── State | Character | OverClock ─────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Character_OverClock, "State.Character.OverClock");
	
	// ── State | Character | Etc ───────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Dead, "State.Character.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_StaminaExhausted, "State.Character.StaminaExhausted"); 
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Stunned, "State.Character.Stunned");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Invincible, "State.Character.Invincible");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_HitReacting, "State.Character.HitReacting");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_NoStaminaRegen, "State.Character.NoStaminaRegen");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_LockOn, "State.Character.LockOn");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_OnPlatform, "State.Character.OnPlatform");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_PotionCoolDown, "State.Character.PotionCoolDown");
	
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Guard, "State.Character.Guard");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Guard_Blocking, "State.Character.Guard.Blocking");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Guard_Break, "State.Character.Guard.Break");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Guard_PerfectGuard, "State.Character.Guard.PerfectGuard");
	
	UE_DEFINE_GAMEPLAY_TAG(State_Character_WeaponDrawn, "State.Character.WeaponDrawn");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_SuperArmor, "State.Character.SuperArmor");

	// ─── Enemy Attack ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Attack_Normal, "State.Enemy.Attack.Normal");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Attack_FirstSkill, "State.Enemy.Attack.FirstSkill");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Attack_CounterAttack, "State.Enemy.Attack.CounterAttack");

	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Hit_Normal, "State.Enemy.Hit.Normal");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Dead, "State.Enemy.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Parried, "State.Enemy.Parried");
	
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH01_Attack_GroundSlap, "State.BossCH01.Attack.GroundSlap");
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH01_Attack_DoubleSweep, "State.BossCH01.Attack.DoubleSweep");
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH01_Attack_GroundHitL, "State.BossCH01.Attack.GroundHitL");
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH01_Attack_GroundHitR, "State.BossCH01.Attack.GroundHitR");
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH01_Attack_Shockwave, "State.BossCH01.Attack.Shockwave");
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH01_Attack_Dash, "State.BossCH01.Attack.Dash");
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH01_Attack_GimmickDash, "State.BossCH01.Attack.GimmickDash");
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH01_Attack_Jump, "State.BossCH01.Attack.Jump");
	
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH02_Attack_Smash, "State.BossCH02.Attack.Smash");
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH02_Attack_SmashSlam, "State.BossCH02.Attack.SmashSlam");
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH02_Attack_GroundExplosion, "State.BossCH02.Attack.GroundExplosion");
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH02_Attack_GroundFall, "State.BossCH02.Attack.GroundFall");
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH02_Attack_ScatterFlame, "State.BossCH02.Attack.ScatterFlame");
	UE_DEFINE_GAMEPLAY_TAG(State_BossCH02_Attack_Jump, "State.BossCH02.Attack.Jump");
	
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Attacking, "State.Boss.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Dashing, "State.Boss.Dashing");
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Dead, "State.Boss.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_Groggy, "State.Boss.Groggy");
	UE_DEFINE_GAMEPLAY_TAG(State_Boss_InGroggy, "State.Boss.InGroggy");
}
