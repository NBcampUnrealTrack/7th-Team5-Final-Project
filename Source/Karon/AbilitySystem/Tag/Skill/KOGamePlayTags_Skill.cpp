#include "KOGamePlayTags_Skill.h"


namespace KOGameplayTags
{
	// ─── Skill Identity (FKOSkillRow::SkillTag) ────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Skill_DimensionSever,    "Skill.DimensionSever");
	UE_DEFINE_GAMEPLAY_TAG(Skill_PlungeCharge,		"Skill.PlungeCharge");
	UE_DEFINE_GAMEPLAY_TAG(Skill_DiveSlash,			"Skill.DiveSlash");
	UE_DEFINE_GAMEPLAY_TAG(Skill_FlashStrike,		"Skill.FlashStrike");
	UE_DEFINE_GAMEPLAY_TAG(Skill_BladeDance,		"Skill.BladeDance");
	
	UE_DEFINE_GAMEPLAY_TAG(Skill_PassiveAtrtack1,	"Skill.PassiveAttack1");
	
	// ─── Skill State (Active Skill Unlocked from UI) ────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Skill_Unlocked_DimensionSever, "Skill.Unlocked.DimensionSever");
	UE_DEFINE_GAMEPLAY_TAG(Skill_Unlocked_SkillTest,      "Skill.Unlocked.SkillTest");
}
