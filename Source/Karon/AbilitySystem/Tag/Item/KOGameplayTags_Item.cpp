#include "KOGameplayTags_Item.h"


namespace KOGameplayTags
{
	// ─── Factory Category ─────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Factory_Boiler,                  "Factory.Boiler");
	UE_DEFINE_GAMEPLAY_TAG(Factory_ModuleDismantler,        "Factory.ModuleDismantler");
	UE_DEFINE_GAMEPLAY_TAG(Factory_AlloyMaker,              "Factory.AlloyMaker");
	UE_DEFINE_GAMEPLAY_TAG(Factory_GearPress,               "Factory.GearPress");
	UE_DEFINE_GAMEPLAY_TAG(Factory_PipeWorkshop,            "Factory.PipeWorkshop");
	
	UE_DEFINE_GAMEPLAY_TAG(Factory_UndergroundMiningModule, "Factory.UndergroundMiningModule");
	UE_DEFINE_GAMEPLAY_TAG(Factory_WeaponWorkbench,         "Factory.WeaponWorkbench");
	
	UE_DEFINE_GAMEPLAY_TAG(Factory_PressurePipe,			"Factory.PressurePipe");

	// ─── Item Category ────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Module,   "Item.Category.Module");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Coal,     "Item.Category.Coal");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Resource, "Item.Category.Resource");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Material, "Item.Category.Material");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Weapon,   "Item.Category.Weapon");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_EnergyResource, "Item.Category.EnergyResource");

	// ─── Item Identity (FKOItemRow::ItemTag) ──────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Item_BasicModule, "Item.BasicModule");
	UE_DEFINE_GAMEPLAY_TAG(Item_Coal,        "Item.Coal");
	UE_DEFINE_GAMEPLAY_TAG(Item_CoalDust,    "Item.CoalDust");
	UE_DEFINE_GAMEPLAY_TAG(Item_Copper,      "Item.Copper");
	UE_DEFINE_GAMEPLAY_TAG(Item_Tin,         "Item.Tin");
	UE_DEFINE_GAMEPLAY_TAG(Item_Bronze,      "Item.Bronze");
	UE_DEFINE_GAMEPLAY_TAG(Item_BronzePlate, "Item.BronzePlate");
	UE_DEFINE_GAMEPLAY_TAG(Item_MiningPipe,  "Item.MiningPipe");
	UE_DEFINE_GAMEPLAY_TAG(Item_DamagedMiningPipe,  "Item.DamagedMiningPipe");
	UE_DEFINE_GAMEPLAY_TAG(Item_Gear,        "Item.Gear");
	UE_DEFINE_GAMEPLAY_TAG(Item_BronzeSword, "Item.BronzeSword");
}
