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
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_QuickSlotBar,      "UI.Widget.QuickSlotBar");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_Boss_HealthBar, "UI.Widget.Boss.HealthBar");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_OverclockProgressBar,		"UI.Widget.OverclockProgressBar");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_DropItem,		"UI.Widget.DropItem");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_TitleMenu,      "UI.Widget.TitleMenu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_BeltConnect,    "UI.Widget.BeltConnect");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_KeyNotice,    "UI.Widget.KeyNotice");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_LevelSequenceText,    "UI.Widget.LevelSequenceText");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_CreditText,    "UI.Widget.CreditText");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_InteractionNotice,    "UI.Widget.InteractionNotice");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_Map_MainMap,    "UI.Widget.Map.MainMap");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_InGameHUD,         "UI.Widget.InGameHUD");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_PlayerMenu,		"UI.Widget.PlayerMenu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_BuildInventory,	"UI.Widget.BuildInventory");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_Option,            "UI.Widget.Option");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_ConfirmationPopup, "UI.Widget.ConfirmationPopup");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_GameOverMenu,		"UI.Widget.GameOverMenu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Widget_Video,		"UI.Widget.Video");
}
