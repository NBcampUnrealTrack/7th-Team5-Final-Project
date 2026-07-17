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
	UE_DEFINE_GAMEPLAY_TAG(Factory_StraightBelt,            "Factory.StraightBelt");
	UE_DEFINE_GAMEPLAY_TAG(Factory_CornerBelt,              "Factory.CornerBelt");
	
	UE_DEFINE_GAMEPLAY_TAG(Factory_PressurePipe,			"Factory.PressurePipe");
	UE_DEFINE_GAMEPLAY_TAG(Factory_RollingMill,				"Factory.RollingMill");
	
	UE_DEFINE_GAMEPLAY_TAG(Factory_HerbFarm,				"Factory.HerbFarm");
	UE_DEFINE_GAMEPLAY_TAG(Factory_JuicePress,				"Factory.JuicePress");
	UE_DEFINE_GAMEPLAY_TAG(Factory_PotionBrewer,			"Factory.PotionBrewer");

	// ─── Item Category ────────────────────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Module,   "Item.Category.Module");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Resource, "Item.Category.Resource");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Material, "Item.Category.Material");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Equipment,   "Item.Category.Equipment");
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Consumable,   "Item.Category.Consumable");
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
	UE_DEFINE_GAMEPLAY_TAG(Item_Sword, "Item.Sword");
	UE_DEFINE_GAMEPLAY_TAG(Item_Hameer, "Item.Hammer");
	UE_DEFINE_GAMEPLAY_TAG(Item_GreatSword, "Item.GreatSword");
	UE_DEFINE_GAMEPLAY_TAG(Item_Head, "Item.Head");
	UE_DEFINE_GAMEPLAY_TAG(Item_UpperBody, "Item.UpperBody");
	UE_DEFINE_GAMEPLAY_TAG(Item_LowerBody, "Item.LowerBody");
	UE_DEFINE_GAMEPLAY_TAG(Item_Shoes, "Item.Shoes");
	UE_DEFINE_GAMEPLAY_TAG(Item_BasicGunpowder, "Item.BasicGunpowder");
	UE_DEFINE_GAMEPLAY_TAG(Item_CopperPlate, "Item.CopperPlate");
	UE_DEFINE_GAMEPLAY_TAG(Item_Sulfur,      "Item.Sulfur");
	UE_DEFINE_GAMEPLAY_TAG(Item_HerbSeed,       "Item.HerbSeed");
	UE_DEFINE_GAMEPLAY_TAG(Item_Herb,			"Item.Herb");
	UE_DEFINE_GAMEPLAY_TAG(Item_HerbJuice,      "Item.HerbJuice");
	UE_DEFINE_GAMEPLAY_TAG(Item_HealingPotion,  "Item.HealingPotion");
}
