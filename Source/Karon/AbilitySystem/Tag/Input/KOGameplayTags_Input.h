#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"


namespace KOGameplayTags
{
	// ───────────────────────── Input ──────────────────────────────
	// ── Input | Native ────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Look);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Interact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_ToggleBuildMode);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Build_Confirm);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Build_ToggleDestroy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_ToggleInventory);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Build_Cancel);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Build_Escape);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Build_QuickSlot1);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Build_QuickSlot2);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_ToggleSKillTree);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Build_QuickSlot3);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Build_QuickSlot4);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Build_QuickSlot5);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Build_Rotate);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_ToggleMap);
	
	// ── Input | Ability | Movement ────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Movement_Jump);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Movement_Sprint);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Movement_Roll);

    // ── Input | Ability | Attack ──────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Combo);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Light);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Heavy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Charge);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Dash);

    // ── Input | Ability | Interaction ─────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Interaction);
	
	// ── Input | Ability | LockOn ─────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_LockOn);
    // ── Input | Ability | CoreSkill ───────────────────────────────
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_CoreSkill_);

}
