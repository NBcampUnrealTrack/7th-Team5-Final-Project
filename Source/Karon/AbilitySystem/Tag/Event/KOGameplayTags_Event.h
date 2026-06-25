#pragma once
#include "NativeGameplayTags.h"


namespace KOGameplayTags
{
	// ───────────────────────── Event ──────────────────────────────
	// ── Event | Stamina ──────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Stamina_Exhausted);
	
	// ── Event | Movement ──────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Movement_Jump_LockOn); 
	
	// ── Event | HitReact ──────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Hit);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact_Forward);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact_Backward);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact_Left);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact_Right);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact_KnockBack_Launch);
	// ── Event | Death ─────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Death);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_Finish);

	// ── Event | Attack ────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Input_Light);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Input_Heavy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Plunge_Land);
	
	// ── Event | OverClock ─────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_OverClock_Start);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_OverClock_End);
	
	// ── Event | Combo ─────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combo_Transition);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combo_EnableInput);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combo_Window_Open);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combo_Window_Close);
	
	// ── Event | Weapon ─────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Weapon_Draw);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Weapon_Sheathe);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Weapon_Equip);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Weapon_Unequip);
	
	// ── Event | Weapon Trace ───────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Trace_Start);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Trace_End);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Dodge_Invincible_Start);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Dodge_Invincible_End);
	
	// ─── Enemy Event ──────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_SkillHit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_DropItem);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Interaction);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_SyncOverclockProgressBar);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss01_Shockwave);
}
