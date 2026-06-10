#include "KOGamePlayTags_Skill.h"


namespace KOGameplayTags
{
	// ─── Skill Identity (FKOSkillRow::SkillTag) ────────────────────────────────
	UE_DEFINE_GAMEPLAY_TAG(Skill_BaseSkill,       "Skill.BaseSkill");
	UE_DEFINE_GAMEPLAY_TAG(Skill_UnlockableSkill, "Skill.UnlockableSkill");
	UE_DEFINE_GAMEPLAY_TAG(Skill_lockedSkill,     "Skill.lockedSkill");
}
