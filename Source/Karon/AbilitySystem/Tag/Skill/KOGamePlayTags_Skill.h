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
	/** 디멘션 세버*/
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_DimensionSever);
	/** 플런지 차지*/
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_PlungeCharge);
	/** 다이브 슬래시*/
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_DiveSlash);
	/** 플래시 스트라이크*/
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_FlashStrike);
	
	/** 패시브 공격력1*/
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_PassiveAtrtack1);
	
	// ─── Skill State (Active Skill Unlocked from UI) ────────────────────────────
	/** 디멘션세버*/
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_Unlocked_DimensionSever);
	/** 단순 테스트용 스킬*/
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Skill_Unlocked_SkillTest);
}
