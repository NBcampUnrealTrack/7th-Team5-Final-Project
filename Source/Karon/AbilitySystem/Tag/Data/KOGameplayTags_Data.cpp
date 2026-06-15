#include "KOGameplayTags_Data.h"

namespace KOGameplayTags
{
	// ───────────────────────── Data ───────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Health, "Data.Attribute.Health");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Health_Damage, "Data.Attribute.Health.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Health_Healing, "Data.Attribute.Health.Healing");
	
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Stamina, "Data.Attribute.Stamina");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Movement, "Data.Attribute.Movement");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Movement_WalkSpeed, "Data.Attribute.Movement.WalkSpeed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Attribute_Combat, "Data.Attribute.Combat");

	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime, "Data.CoolTime");
	UE_DEFINE_GAMEPLAY_TAG(Data_DebuffTime, "Data.DebuffTime");
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
	
	// ───────────────────────── Boss Cool Time Data ────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH01_GroundSlap, "Data.CoolTime.BossCH01.GroundSlap");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH01_DoubleGroundHit, "Data.CoolTime.BossCH01.DoubleGroundHit");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH01_GroundHitL, "Data.CoolTime.BossCH01.GroundHit");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH01_ShockWave, "Data.CoolTime.BossCH01.ShockWave");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime_BossCH01_Dash, "Data.CoolTime.BossCH01.Dash");
	
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
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Build_ModeChanged, "Data.Message.Build.ModeChanged");

}
