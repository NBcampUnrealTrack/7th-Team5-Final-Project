#include "KOGameplayTags_Event.h"


namespace KOGameplayTags
{
	// ───────────────────────── Event ─────────────────────────────
	// ── Event | Stamina ──────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Stamina_Exhausted, "Event.Stamina.Exhausted");
	
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
	
	// ── Event | Combo ─────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Window_Open, "Event.Combo.Window.Open");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Window_Close, "Event.Combo.Window.Close");
	
	// ─── Enemy Event ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_SkillHit, "Event.SkillHit");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Boss01_Shockwave, "Event.Boss1.Shockwave");
}
