#include "KOGameplayTags_Data.h"

namespace KOGameplayTags
{
	// ───────────────────────── Data ───────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Health, "Data.Attribute.Health");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Health_Current, "Data.Attribute.Health.Current");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Health_Max, "Data.Attribute.Health.Max");
	
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_Healing, "Data.Healing");
	
	UE_DEFINE_GAMEPLAY_TAG(Data_AttackCoefficient, "Data.AttackCoefficient"); 
	
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Stamina, "Data.Attribute.Stamina");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Stamina_Current, "Data.Attribute.Stamina.Current");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Stamina_Max, "Data.Attribute.Stamina.Max");
	
	UE_DEFINE_GAMEPLAY_TAG(Data_StaminaDrain, "Data.StaminaDrain");
	UE_DEFINE_GAMEPLAY_TAG(Data_StaminaRegen, "Data.StaminaRegen");
	
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Movement, "Data.Attribute.Movement");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Movement_WalkSpeed, "Data.Attribute.Movement.WalkSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Movement_MaxAcceleration, "Data.Attribute.Movement.MaxAcceleration");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Movement_GroundFriction, "Data.Attribute.Movement.GroundFriction");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Movement_BrakingDeceleration, "Data.Attribute.Movement.BrakingDeceleration");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Movement_MaxWalkSpeedCrouch, "Data.Attribute.Movement.MaxWalkSpeedCrouch");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Movement_JumpStrength, "Data.Attribute.Movement.JumpStrength");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Movement_GravityScale, "Data.Attribute.Movement.GravityScale");
	
	
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Combat, "Data.Attribute.Combat");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Combat_AttackPower, "Data.Attribute.Combat.AttackPower");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Combat_AttackSpeed, "Data.Attribute.Combat.AttackSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Combat_CritChance, "Data.Attribute.Combat.CritChance");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Combat_CritMultiplier, "Data.Attribute.Combat.CritMultiplier");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Combat_Defense, "Data.Attribute.Combat.Defense");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Combat_Clock, "Data.Attribute.Combat.Clock");
	
	
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime, "Data.CoolTime");
	UE_DEFINE_GAMEPLAY_TAG(Data_Skill_CoolTime, "Data.Skill.CoolTime");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_DiveSlash, "Data.CoolTime.DiveSlash");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_DimensionSever, "Data.CoolTime.DimensionSever");
	UE_DEFINE_GAMEPLAY_TAG(Data_DebuffTime, "Data.DebuffTime");
	
	
	// ───────────────────────── Boss Cool Time Data ────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH01_GroundSlap, "Data.CoolTime.BossCH01.GroundSlap");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH01_DoubleSweep, "Data.CoolTime.BossCH01.DoubleSweep");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH01_GroundHitL, "Data.CoolTime.BossCH01.GroundHit");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH01_ShockWave, "Data.CoolTime.BossCH01.ShockWave");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH01_Dash, "Data.CoolTime.BossCH01.Dash");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH01_Jump, "Data.CoolTime.BossCH01.Jump");
	
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH02_Smash, "Data.CoolTime.BossCH02.Smash");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH02_SmashSlam, "Data.CoolTime.BossCH02.SmashSlam");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH02_GroundExplosion, "Data.CoolTime.BossCH02.GroundExplosion");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH02_GroundFall, "Data.CoolTime.BossCH02.GroundFall");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH02_ScatterFlame, "Data.CoolTime.BossCH02.ScatterFlame");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH02_Jump, "Data.CoolTime.BossCH02.Jump");
	
	
	// ─── Data / Message Channels ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Inventory_Changed,    "Data.Message.Inventory.Changed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Factory_StateChanged, "Data.Message.Factory.StateChanged");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_UI_OpenWidget,        "Data.Message.UI.OpenWidget");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_UI_CloseWidget,       "Data.Message.UI.CloseWidget");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Building_Interacted,  "Data.Message.Building.Interacted");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Build_QuickSlotChanged, "Data.Message.Build.QuickSlotChanged");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Producer_FuelChanged,   "Data.Message.Producer.FuelChanged");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Processor_Changed,      "Data.Message.Processor.Changed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Build_QuickSlotSelectionChanged, "Data.Message.Build.QuickSlotSelectionChanged");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Build_ModeChanged,          "Data.Message.Build.ModeChanged");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Skill_QuickSlotChanged,     "Data.Message.Skill.QuickSlotChanged");

}
