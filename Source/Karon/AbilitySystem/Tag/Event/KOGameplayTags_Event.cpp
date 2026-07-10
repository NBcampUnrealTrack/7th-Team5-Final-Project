#include "KOGameplayTags_Event.h"


namespace KOGameplayTags
{
	// ───────────────────────── Event ─────────────────────────────
	// ── Event | Stamina ──────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Stamina_Exhausted, "Event.Stamina.Exhausted");
	
	// ── Event | Movement ──────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Movement_Jump_LockOn, "Event.Movement.Jump.LockOn");
	
	// ── Event | Hit ───────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Hit, "Event.Hit");
	
	// ── Event | HitReact ──────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReact, "Event.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReact_Forward, "Event.HitReact.Forward");
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReact_Backward, "Event.HitReact.Backward");
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReact_Left, "Event.HitReact.Left");
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReact_Right, "Event.HitReact.Right");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReact_KnockBack_Launch, "Event.HitReact.KnockBack_Launch"); 
	// ── Event | Death ─────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Death, "Event.Death");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Montage_Finish, "Event.Montage.Finish");
	UE_DEFINE_GAMEPLAY_TAG(Event_SwapSocket, "Event.SwapSocket");
	UE_DEFINE_GAMEPLAY_TAG(Event_Stamina_Full, "Event.Stamina.Full");
	
	// ── Event | Attack ────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Light, "Event.Input.Light");
	UE_DEFINE_GAMEPLAY_TAG(Event_Input_Heavy, "Event.Input.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(Event_Plunge_Land, "Event.Plunge.Land");
	UE_DEFINE_GAMEPLAY_TAG(Event_ParryAttack, "Event.ParryAttack");
	
	// ── Event | OverClock ─────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_OverClock_Start, "Event.OverClock.Start");
	UE_DEFINE_GAMEPLAY_TAG(Event_OverClock_End, "Event.OverClock.End");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Clock_Gain, "Event.Clock.Gain");
	
	// ── Event | Combo ─────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Transition, "Event.Combo.Transition");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_EnableInput, "Event.Combo.EnableInput");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Window_Open, "Event.Combo.Window.Open");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Window_Close, "Event.Combo.Window.Close");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Draw, "Event.Weapon.Draw");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Sheathe, "Event.Weapon.Sheathe");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_ShouldDraw, "Event.Weapon.ShouldDraw");
	
	// ── Event | Weapon ─────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Equip,   "Event.Weapon.Equip");
	UE_DEFINE_GAMEPLAY_TAG(Event_Weapon_Unequip, "Event.Weapon.Unequip");
	
	// ── Event | Weapon Trace ───────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Trace_Start,   "Event.Trace.Start");
	UE_DEFINE_GAMEPLAY_TAG(Event_Trace_End, "Event.Trace.End");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Dodge_Invincible_Start, "Event.Dodge.Invincible.Start");
	UE_DEFINE_GAMEPLAY_TAG(Event_Dodge_Invincible_End, "Event.Dodge.Invincible.End");

	// ── Event | Guard ─────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Guard_Start, "Event.Guard.Start");
	UE_DEFINE_GAMEPLAY_TAG(Event_Guard_End, "Event.Guard.End");
	UE_DEFINE_GAMEPLAY_TAG(Event_Guard_Break, "Event.Guard.Break");
	UE_DEFINE_GAMEPLAY_TAG(Event_Guard_DirectionFail, "Event.Guard.DirectionFail");
	UE_DEFINE_GAMEPLAY_TAG(Event_Guard_Success, "Event.Guard.Success");
	UE_DEFINE_GAMEPLAY_TAG(Event_PerfectGuard_Start, "Event.PerfectGuard.Start");
	UE_DEFINE_GAMEPLAY_TAG(Event_PerfectGuard_End, "Event.PerfectGuard.End");
	
	// ─── Enemy Event ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_SkillHit, "Event.SkillHit");
	UE_DEFINE_GAMEPLAY_TAG(Event_DropItem, "Event.DropItem");
	UE_DEFINE_GAMEPLAY_TAG(Event_Interaction, "Event.Interaction");
	UE_DEFINE_GAMEPLAY_TAG(Event_Damaged, "Event.Damaged");
	UE_DEFINE_GAMEPLAY_TAG(Event_Parried, "Event.Parried");
	UE_DEFINE_GAMEPLAY_TAG(Event_CounterAttack, "Event.CounterAttack");
	UE_DEFINE_GAMEPLAY_TAG(Event_Telemetry_Combat, "Event.Telemetry.Combat");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_Air_GroundImpact, "Event.Attack.Air.GroundImpact");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_SyncOverclockProgressBar, "Event.SyncOverclockProgressBar");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Jump_Launch, "Event.Jump.Launch");
	UE_DEFINE_GAMEPLAY_TAG(Event_Boss01_Shockwave, "Event.Boss1.Shockwave");
	UE_DEFINE_GAMEPLAY_TAG(Event_Boss02_ScatterFire, "Event.Boss2.ScatterFire");
}
