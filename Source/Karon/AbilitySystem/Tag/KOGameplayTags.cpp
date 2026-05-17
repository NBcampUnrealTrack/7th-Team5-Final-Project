#include "KOGameplayTags.h"

namespace KOGameplayTags
{
	// ─── Input ───────────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Move, "Input.Native.Move");
	UE_DEFINE_GAMEPLAY_TAG(Input_Native_Look, "Input.Native.Look");

	// ─── UI Layer ─────────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Game,     "UI.Layer.Game");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_GameMenu, "UI.Layer.GameMenu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Menu,     "UI.Layer.Menu");
	UE_DEFINE_GAMEPLAY_TAG(UI_Layer_Modal,    "UI.Layer.Modal");

	// ─── Message Channels ─────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Message_Inventory_Changed,    "Message.Inventory.Changed");
	UE_DEFINE_GAMEPLAY_TAG(Message_Factory_StateChanged, "Message.Factory.StateChanged");
	UE_DEFINE_GAMEPLAY_TAG(Message_UI_PushLayerRequest,  "Message.UI.PushLayerRequest");
}
