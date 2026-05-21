#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace KOGameplayTags
{
	// ───────────────────────── Input ──────────────────────────────
	// ── Input | Native ────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Look);
	
	// ── Input | Ability | Movement ────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Movement_Jump);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Movement_Sprint);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Movement_Roll);

    // ── Input | Ability | Attack ──────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Combo);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Heavy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Charge);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Dash);

    // ── Input | Ability | Interaction ─────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Interaction);
	
	// ── Input | Ability | LockOn ─────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_LockOn);
    // ── Input | Ability | CoreSkill ───────────────────────────────
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_CoreSkill_);

	
	// ───────────────────────── State ──────────────────────────────
    // ── State | Character | Movement ──────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Movement_InAir);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Movement_Sprinting);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Movement_Rolling);

    // ── State | Character | Attacking ─────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking);
	//TODO:
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking_Combo);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking_Heavy);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking_Charge);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking_Dash);

    // ── State | Character | Etc ───────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Dead);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Stunned);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Invincible);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_NoStaminaRegen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_LockOn);
	
	// ───────────────────────── Event ──────────────────────────────
    // ── Event | HitReact ──────────────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact);

    // ── Event | Death ─────────────────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Death);

    // ── Event | Combo ─────────────────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combo_Window_Open);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combo_Window_Close);

	
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

	// ───────────────────────── Data ───────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Health);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Stamina);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement_WalkSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Combat); 
	
	
}
