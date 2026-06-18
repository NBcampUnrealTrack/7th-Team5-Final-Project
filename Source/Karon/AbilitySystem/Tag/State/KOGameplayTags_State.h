#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"


namespace KOGameplayTags
{
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
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_StaminaExhausted);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Stunned);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Invincible);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_NoStaminaRegen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_LockOn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_OnPlatform); // 보스 기믹용 태그 
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_WeaponDrawn)
	
	// ─── Enemy Attack ──────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Enemy_Attack_Normal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Enemy_Attack_FirstSkill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Enemy_Hit_Normal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Enemy_Dead);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_BossCH01_Attack_GroundSlap);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_BossCH01_Attack_DoubleSweep);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_BossCH01_Attack_GroundHitL);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_BossCH01_Attack_GroundHitR);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_BossCH01_Attack_Shockwave);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_BossCH01_Attack_Dash);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_BossCH01_Attack_GimmickDash);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_BossCH01_Attack_Jump);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Boss_Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Boss_Dashing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Boss_Dead);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Boss_Groggy);


	
	
}
