#include "KOGameplayTags.h"

namespace KOGameplayTags
{
	// ─── Input ───────────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Move,     "Input.Native.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Look,     "Input.Native.Look");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Interact, "Input.Native.Interact");

	// ─── UI Layer ─────────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Game,     "UI.Layer.Game");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_GameMenu, "UI.Layer.GameMenu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Menu,     "UI.Layer.Menu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Modal,    "UI.Layer.Modal");

	// ─── Data / Message Channels ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Inventory_Changed,    "Data.Message.Inventory.Changed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Factory_StateChanged, "Data.Message.Factory.StateChanged");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_UI_PushLayerRequest,  "Data.Message.UI.PushLayerRequest");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Building_Interacted,  "Data.Message.Building.Interacted");

	// ─── Item Category ────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Module,   "Item.Category.Module");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Coal,     "Item.Category.Coal");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Resource, "Item.Category.Resource");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Material, "Item.Category.Material");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Weapon,   "Item.Category.Weapon");
	
	// ─── SetbyCaller ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime, "Data.CoolTime");
	
	// ─── Enemy Event ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_SkillHit, "Event.SkillHit");
	
	// ─── Enemy Attack ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Attack_Normal, "State.Enemy.Attack.Normal");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Attack_FirstSkill, "State.Enemy.Attack.FirstSkill");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Hit_Normal, "State.Enemy.Hit.Normal");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Dead, "State.Enemy.Dead");
}
