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
	UE_DEFINE_GAMEPLAY_TAG(Item_Category_Core, "Item.Category.Core");

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
	UE_DEFINE_GAMEPLAY_TAG(Item_Hammer, "Item.Hammer");
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
	
	// --- Sword ---
	UE_DEFINE_GAMEPLAY_TAG(Item_Sword_Basic, "Item.Sword.Basic");
	UE_DEFINE_GAMEPLAY_TAG(Item_Sword_Earth, "Item.Sword.Earth");
	UE_DEFINE_GAMEPLAY_TAG(Item_Sword_Water, "Item.Sword.Water");
	UE_DEFINE_GAMEPLAY_TAG(Item_Sword_Lightning, "Item.Sword.Lightning");

	// --- Hammer ---
	UE_DEFINE_GAMEPLAY_TAG(Item_Hammer_Basic, "Item.Hammer.Basic");
	UE_DEFINE_GAMEPLAY_TAG(Item_Hammer_Earth, "Item.Hammer.Earth");
	UE_DEFINE_GAMEPLAY_TAG(Item_Hammer_Water, "Item.Hammer.Water");
	UE_DEFINE_GAMEPLAY_TAG(Item_Hammer_Lightning, "Item.Hammer.Lightning");

	// --- Head ---
	UE_DEFINE_GAMEPLAY_TAG(Item_Head_Basic, "Item.Head.Basic");
	UE_DEFINE_GAMEPLAY_TAG(Item_Head_Earth, "Item.Head.Earth");
	UE_DEFINE_GAMEPLAY_TAG(Item_Head_Water, "Item.Head.Water");
	UE_DEFINE_GAMEPLAY_TAG(Item_Head_Lightning, "Item.Head.Lightning");

	// --- UpperBody ---
	UE_DEFINE_GAMEPLAY_TAG(Item_UpperBody_Basic, "Item.UpperBody.Basic");
	UE_DEFINE_GAMEPLAY_TAG(Item_UpperBody_Earth, "Item.UpperBody.Earth");
	UE_DEFINE_GAMEPLAY_TAG(Item_UpperBody_Water, "Item.UpperBody.Water");
	UE_DEFINE_GAMEPLAY_TAG(Item_UpperBody_Lightning, "Item.UpperBody.Lightning");

	// --- LowerBody ---
	UE_DEFINE_GAMEPLAY_TAG(Item_LowerBody_Basic, "Item.LowerBody.Basic");
	UE_DEFINE_GAMEPLAY_TAG(Item_LowerBody_Earth, "Item.LowerBody.Earth");
	UE_DEFINE_GAMEPLAY_TAG(Item_LowerBody_Water, "Item.LowerBody.Water");
	UE_DEFINE_GAMEPLAY_TAG(Item_LowerBody_Lightning, "Item.LowerBody.Lightning");

	// --- Shoes ---
	UE_DEFINE_GAMEPLAY_TAG(Item_Shoes_Basic, "Item.Shoes.Basic");
	UE_DEFINE_GAMEPLAY_TAG(Item_Shoes_Earth, "Item.Shoes.Earth");
	UE_DEFINE_GAMEPLAY_TAG(Item_Shoes_Water, "Item.Shoes.Water");
	UE_DEFINE_GAMEPLAY_TAG(Item_Shoes_Lightning, "Item.Shoes.Lightning");
	
	UE_DEFINE_GAMEPLAY_TAG(Item_Earth,  "Item.Earth");
	UE_DEFINE_GAMEPLAY_TAG(Item_Water,  "Item.Water");
	UE_DEFINE_GAMEPLAY_TAG(Item_Fire,  "Item.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Item_Lightning,  "Item.Lightning");
}
