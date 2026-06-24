#include "KOGamePlayTags_Skill.h"


namespace KOGameplayTags
{
	// ─── Skill Identity (FKOSkillRow::SkillTag) ────────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Skill_BaseSkill,         "Skill.BaseSkill");
	UE_DEFINE_GAMEPLAY_TAG(Skill_UnlockableSkill,   "Skill.UnlockableSkill");
	UE_DEFINE_GAMEPLAY_TAG(Skill_lockedSkill,       "Skill.lockedSkill");
	UE_DEFINE_GAMEPLAY_TAG(Skill_DimensionSever,    "Skill.DimensionSever");
	
	// ─── Skill State (Active Skill Unlocked from UI) ────────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Skill_Unlocked_DimensionSever, "Skill.Unlocked.DimensionSever");
	UE_DEFINE_GAMEPLAY_TAG(Skill_Unlocked_SkillTest,      "Skill.Unlocked.SkillTest");
}
