#pragma once

#include "NativeGameplayTags.h"


namespace KOGameplayTags
{
	// ───────────────────────── Data ───────────────────────────────
	// ───────── Attributes ─────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Health);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Health_Current);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Health_Max);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Healing);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_AttackCoefficient);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Stamina);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Stamina_Current);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Stamina_Max);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_StaminaDrain);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_StaminaRegen);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement_WalkSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement_MaxAcceleration);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement_GroundFriction);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement_BrakingDeceleration);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement_MaxWalkSpeedCrouch);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement_JumpStrength);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement_GravityScale);
	
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Combat);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Combat_AttackPower);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Combat_AttackSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Combat_CritChance);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Combat_CritMultiplier);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Combat_Defense);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Combat_Clock);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Combat_Guard);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_Guard);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Skill_CoolTime);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_LockOn);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_Attack);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_DebuffTime);
	
	// ───────────────────────── Character Skill Cool Time Data ────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_Character_Plunge);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_Character_DimensionSever);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_Character_DiveSlash);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_Character_FlashStrike);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_Character_BladeDance);
	
	// ───────────────────────── Boss Cool Time Data ────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH01_GroundSlap);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH01_DoubleSweep);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH01_GroundHit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH01_ShockWave);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH01_Dash);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH01_Jump);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH02_Smash);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH02_SmashSlam);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH02_GroundExplosion);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH02_GroundFall);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH02_ScatterFlame);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH02_Jump);
	
	// ─── Data / Message Channels ──────────────────────────────────────────────
	/** 인벤토리 변경 메시지 채널 (FKOInventoryChangedMessage 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Inventory_Changed);
	/** 팩토리 상태 변경 메시지 채널 (FKOFactoryStateChangedMessage 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Factory_StateChanged);
	/** UI 위젯 열기 요청 메시지 채널 (FKOUIWidgetRequest 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_UI_OpenWidget);
	/** UI 위젯 닫기 요청 메시지 채널 (FKOUIWidgetRequest 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_UI_CloseWidget);
	/** 건물 상호작용 메시지 채널 (FKOBuildingInteractedMessage 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Building_Interacted);
	/** 건물 퀵슬롯 변경 메시지 채널 (KOBuildQuickSlotWidget 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Build_QuickSlotChanged);
	/** 건설 퀵슬롯 선택 변경 메시지 채널 (KOBuildUIComponent 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Build_QuickSlotSelectionChanged);
	/** 건설 모드 변경 메시지 채널 (KOGridBuildComponent 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Build_ModeChanged);
	/** Producer 연료 변경 메시지 채널 (FKOProducerFuelChangedMessage 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Producer_FuelChanged);
	/** Processor 변경 메시지 채널 (FKOProcessorChangedMessage 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Processor_Changed);
	/** 스킬 퀵슬롯 배정 변경 메시지 채널 (FKOSkillQuickSlotChangedMessage 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Skill_QuickSlotChanged);
}
