#include "KOGameplayTags_UI.h"

namespace KOGameplayTags
{
	// ─── UI Layout (Root) ─────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(UI_Layout_InGame, "UI.Layout.InGame");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layout_Title,  "UI.Layout.Title");

	// ─── UI Layer ─────────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Game,     "UI.Layer.Game");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_GameMenu, "UI.Layer.GameMenu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Menu,     "UI.Layer.Menu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Modal,    "UI.Layer.Modal");

	// ─── UI Widget ────────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_Factory_Processor, "UI.Widget.Factory.Processor");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_Factory_Producer,  "UI.Widget.Factory.Producer");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_Inventory,         "UI.Widget.Inventory");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_QuickSlotBar,      "UI.Widget.QuickSlotBar");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_Boss_HealthBar, "UI.Widget.Boss.HealthBar");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_SkillTree,      "UI.Widget.SkillTree");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_TitleMenu,      "UI.Widget.TitleMenu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_BeltConnect,    "UI.Widget.BeltConnect");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_Map_MainMap,    "UI.Widget.Map.MainMap");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_FactoryCraft,	 "UI.Widget.FactoryCraft");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_InGameHUD,         "UI.Widget.InGameHUD");
}
