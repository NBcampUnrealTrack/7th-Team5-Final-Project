#pragma once

#include "NativeGameplayTags.h"


namespace KOGameplayTags
{
	// ───────────────────────── Data ───────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Health);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Health_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Health_Healing);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Stamina);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement_WalkSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Combat);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_DebuffTime);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	
	// ───────────────────────── Boss Cool Time Data ────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH01_GroundSlap);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH01_DoubleGroundHit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH01_GroundHit);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH01_ShockWave);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime_BossCH01_Dash);
	
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
}
