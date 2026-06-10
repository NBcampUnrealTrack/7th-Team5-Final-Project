#pragma once

#include "NativeGameplayTags.h"

namespace KOGameplayTags
{
	// ─── Skill Identity (FKOSkillRow::SkillTag) ────────────────────────────────
	/** 기본 스킬(테스트용1)*/
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_BaseSkill);
	/** 해제 가능 스킬(테스트용2)*/
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_UnlockableSkill);
	/** 해제 불가능 스킬(테스트용3)*/
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_lockedSkill);
}
