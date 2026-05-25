#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace KOGameplayTags
{
	// ─── Input ───────────────────────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Look);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Interact);

	// ─── UI Layer ─────────────────────────────────────────────────────────────
	/** 게임 플레이 중 항상 표시되는 HUD 레이어 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Game);
	/** 게임 중 메뉴 (일시정지 등) 레이어 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_GameMenu);
	/** 메인 메뉴 레이어 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Menu);
	/** 모달 다이얼로그 레이어 (최상위) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(UI_Layer_Modal);

	// ─── Data / Message Channels ──────────────────────────────────────────────
	/** 인벤토리 변경 메시지 채널 (FKOInventoryChangedMessage 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Inventory_Changed);
	/** 팩토리 상태 변경 메시지 채널 (FKOFactoryStateChangedMessage 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Factory_StateChanged);
	/** UI 레이어 Push 요청 메시지 채널 (FKOUIPushLayerRequest 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_UI_PushLayerRequest);
	/** 건물 상호작용 메시지 채널 (FKOBuildingInteractedMessage 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Building_Interacted);

	// ─── Item Category ────────────────────────────────────────────────────────
	/** 기초 모듈 카테고리 (BasicModule) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Category_Module);
	/** 석탄 카테고리 (Coal) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Category_Coal);
	/** 자원 카테고리 (CoalDust, Copper, Tin, Bronze) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Category_Resource);
	/** 가공 재료 카테고리 (BronzePlate, MiningPipe, Gear 등) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Category_Material);
	/** 무기 카테고리 (BronzeSword) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Category_Weapon);
}
