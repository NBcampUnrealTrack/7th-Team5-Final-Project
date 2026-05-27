#include "KOGameplayTags.h"

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
	
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_ToggleAssignMenu,  "Input.Native.Build.ToggleAssignMenu");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_Cancel,  "Input.Native.Build.Cancel");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_Escape,  "Input.Native.Build.Escape");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_QuickSlot1,  "Input.Native.Build.QuickSlot1");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Build_QuickSlot2,  "Input.Native.Build.QuickSlot2");
	
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
	
	// ── Input | Ability | LockOn
	UE_DEFINE_GAMEPLAY_TAG(Input_Ability_LockOn, "Input.Ability.LockOn");
	
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
	UE_DEFINE_GAMEPLAY_TAG(State_Character_LockOn, "State.Character.LockOn");
	
	// ─── UI Layer ─────────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Game,     "UI.Layer.Game");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_GameMenu, "UI.Layer.GameMenu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Menu,     "UI.Layer.Menu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Modal,    "UI.Layer.Modal");

	// ─── UI Widget ────────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_Factory_Processor, "UI.Widget.Factory.Processor");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_Factory_Producer,  "UI.Widget.Factory.Producer");

	// ─── Data / Message Channels ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Inventory_Changed,    "Data.Message.Inventory.Changed");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Factory_StateChanged, "Data.Message.Factory.StateChanged");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_UI_PushLayerRequest,  "Data.Message.UI.PushLayerRequest");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Building_Interacted,  "Data.Message.Building.Interacted");
	UE_DEFINE_GAMEPLAY_TAG(Data_Message_Build_QuickSlotChanged, "Data.Message.Build.QuickSlotChanged");

	// ─── Factory Category ─────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Factory_Boiler,                  "Factory.Boiler");
	UE_DEFINE_GAMEPLAY_TAG(Factory_ModuleDismantler,        "Factory.ModuleDismantler");
	UE_DEFINE_GAMEPLAY_TAG(Factory_AlloyMaker,              "Factory.AlloyMaker");
	UE_DEFINE_GAMEPLAY_TAG(Factory_GearPress,               "Factory.GearPress");
	UE_DEFINE_GAMEPLAY_TAG(Factory_PipeWorkshop,            "Factory.PipeWorkshop");
	UE_DEFINE_GAMEPLAY_TAG(Factory_UndergroundMiningModule, "Factory.UndergroundMiningModule");
	UE_DEFINE_GAMEPLAY_TAG(Factory_WeaponWorkbench,         "Factory.WeaponWorkbench");

	// ─── Item Category ────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Module,   "Item.Category.Module");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Coal,     "Item.Category.Coal");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Resource, "Item.Category.Resource");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Material, "Item.Category.Material");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Weapon,   "Item.Category.Weapon");
	
	// ─── SetbyCaller ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
	UE_DEFINE_GAMEPLAY_TAG(Data_CoolTime, "Data.CoolTime");
	
	
	// ───────────────────────── Event ──────────────────────────────
	// ── Event | HitReact ──────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReact, "Event.HitReact");
	
	// ── Event | Death ─────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Death, "Event.Death");
	
	// ── Event | Combo ─────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Window_Open, "Event.Combo.Window.Open");
	UE_DEFINE_GAMEPLAY_TAG(Event_Combo_Window_Close, "Event.Combo.Window.Close");
	
	// ─── Enemy Event ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Event_SkillHit, "Event.SkillHit");
	
	// ─── Enemy Attack ──────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Attack_Normal, "State.Enemy.Attack.Normal");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Attack_FirstSkill, "State.Enemy.Attack.FirstSkill");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Hit_Normal, "State.Enemy.Hit.Normal");
	UE_DEFINE_GAMEPLAY_TAG(State_Enemy_Dead, "State.Enemy.Dead");


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

	// ── Enemy Boss ────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Attack_Boss01_Normal01, "Attack.Boss01.Normal01");
	UE_DEFINE_GAMEPLAY_TAG(Attack_Boss01_Normal02, "Attack.Boss01.Normal02");
	UE_DEFINE_GAMEPLAY_TAG(Attack_Boss01_Strong01, "Attack.Boss01.Strong01");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Boss01_Shockwave, "Event.Boss01.Shockwave");
	
	UE_DEFINE_GAMEPLAY_TAG(State_Attacking, "State.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Groggy, "State.Groggy");
	
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Boss01_Normal01, "Cooldown.Boss01.Normal01")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Boss01_Normal02, "Cooldown.Boss01.Normal02")
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Boss01_Strong01, "Cooldown.Boss01.Strong01")
}
