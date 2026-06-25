#pragma once

#include "NativeGameplayTags.h"

namespace KOGameplayTags
{
	//── UI Layout (Root) ─────────────────────────────────────────────────────
	/** 인게임 루트 레이아웃 컨텍스트 (UKOUISettings::RootLayoutMap 키) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layout_InGame);
	/** 타이틀 루트 레이아웃 컨텍스트 (UKOUISettings::RootLayoutMap 키) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layout_Title);

	//── UI Layer ─────────────────────────────────────────────────────────────
	/** 게임 플레이 중 항상 표시되는 HUD 레이어 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Game);
	/** 게임 중 메뉴 (일시정지 등) 레이어 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_GameMenu);
	/** 메인 메뉴 레이어 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Menu);
	/** 모달 다이얼로그 레이어 (최상위) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Modal);

	//── UI Widget ────────────────────────────────────────────────────────────
	/** Processor 설비 상호작용 위젯 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_Factory_Processor);
	/** Producer 설비 상호작용 위젯 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_Factory_Producer);
	/** 퀵슬롯 위젯 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_QuickSlotBar);
	
	//** 보스 체력바 위젯 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_Boss_HealthBar);
	//** 아이템 드랍 위젯 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_DropItem);
	/** 스킬 트리 팝업 위젯 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_SkillTree);
	/** 타이틀 메뉴 위젯 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_TitleMenu);
	/** 벨트-공장 포트 연결 팝업 위젯 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_BeltConnect);
	
	/** 키조작을 알려주는 위젯 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_KeyNotice);
	
	/** 상호작용을 알려주는 위젯 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_InteractionNotice);
	
	/** 맵 위젯 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_Map_MainMap);
	
	/** 메인 HUD */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_InGameHUD);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Widget_PlayerMenu);
}
