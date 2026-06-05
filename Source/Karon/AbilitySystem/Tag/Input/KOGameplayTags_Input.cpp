#include "KOGameplayTags_Input.h"

namespace KOGameplayTags
{
	// ───────────────────────── Input ──────────────────────────────
	// ── Input | Native ────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Move,     "Input.Native.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Look,     "Input.Native.Look");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Interact, "Input.Native.Interact");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_ToggleBuildMode,     "Input.Native.ToggleBuildMode");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_Confirm,        "Input.Native.Build.Confirm");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_ToggleDestroy,  "Input.Native.Build.ToggleDestroy");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_ToggleInventory,      "Input.Native.ToggleInventory");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_Cancel,  "Input.Native.Build.Cancel");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_Escape,  "Input.Native.Build.Escape");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_QuickSlot1,  "Input.Native.Build.QuickSlot1");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_QuickSlot2,  "Input.Native.Build.QuickSlot2");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_QuickSlot3,  "Input.Native.Build.QuickSlot3");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_QuickSlot4,  "Input.Native.Build.QuickSlot4");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_QuickSlot5,  "Input.Native.Build.QuickSlot5");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_ToggleSKillTree,  "Input.Native.ToggleSkillTree");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_Rotate,  "Input.Native.Build.Rotate");
	
	
	// ── Input | Ability | Movement ────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Movement_Jump, "Input.Ability.Movement.Jump");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Movement_Sprint, "Input.Ability.Movement.Sprint");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Movement_Roll, "Input.Ability.Movement.Roll");
	
	// ── Input | Ability | Attack ──────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Attack_Combo, "Input.Ability.Attack.Combo");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Attack_Light, "Input.Ability.Attack.Light");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Attack_Heavy, "Input.Ability.Attack.Heavy");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Attack_Charge, "Input.Ability.Attack.Charge");
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Attack_Dash, "Input.Ability.Attack.Dash");
	
	// ── Input | Ability | Interaction ─────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_Interaction, "Input.Ability.Interaction");
	
	// ── Input | Ability | LockOn
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_LockOn, "Input.Ability.LockOn");
	
	// ── Input | Ability | CoreSkill ───────────────────────────────
}
