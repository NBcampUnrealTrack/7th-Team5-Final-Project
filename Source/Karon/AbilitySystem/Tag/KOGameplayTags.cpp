#include "KOGameplayTags.h"

namespace KOGameplayTags
{
	// ───────────────────────── Input ──────────────────────────────
	// ── Input | Native ────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Move, "Input.Native.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Look, "Input.Native.Look");
	
	// ── Input | Ability | Movement ────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Movement_Jump, "Input.Ability.Movement.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Movement_Sprint, "Input.Ability.Movement.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Movement_Roll, "Input.Ability.Movement.Roll");
	
	// ── Input | Ability | Attack ──────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Attack_Combo, "Input.Ability.Attack.Combo");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Attack_Heavy, "Input.Ability.Attack.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Attack_Charge, "Input.Ability.Attack.Charge");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Attack_Dash, "Input.Ability.Attack.Dash");
	
	// ── Input | Ability | Interaction ─────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Interaction, "Input.Ability.Interaction");
	
	
	// ── Input | Ability | CoreSkill ───────────────────────────────
	
	
	
	// ───────────────────────── State ──────────────────────────────
	// ── State | Character | Movement ──────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Movement_InAir, "State.Character.Movement.InAir");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Movement_Sprinting, "State.Character.Movement.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Movement_Rolling, "State.Character.Movement.Rolling");
	
	// ── State | Character | Attacking ─────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Attacking, "State.Character.Attacking");
	
	
	// ── State | Character | Etc ───────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Dead, "State.Character.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Stunned, "State.Character.Stunned");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_Invincible, "State.Character.Invincible");
	UE_DEFINE_GAMEPLAY_TAG(State_Character_NoStaminaRegen, "State.Character.NoStaminaRegen");
	
	
	// ───────────────────────── Event ──────────────────────────────
	// ── Event | HitReact ──────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReact, "Event.HitReact");
	
	// ── Event | Death ─────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Death, "Event.Death");
	
	// ── Event | Combo ─────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Window_Open, "Event.Combo.Window.Open");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Window_Close, "Event.Combo.Window.Close");
	
	
	// ───────────────────────── Effect ─────────────────────────────
	// ── Effect | Damage ───────────────────────────────────────────
	
	// ── Effect | Buff ─────────────────────────────────────────────
	
	// ── Effect | Debuff ───────────────────────────────────────────
	
	// ───────────────────────── Data ───────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Health, "Data.Attribute.Health");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Stamina, "Data.Attribute.Stamina");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Movement, "Data.Attribute.Movement");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Movement_WalkSpeed, "Data.Attribute.Movement.WalkSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Combat, "Data.Attribute.Combat");
}
