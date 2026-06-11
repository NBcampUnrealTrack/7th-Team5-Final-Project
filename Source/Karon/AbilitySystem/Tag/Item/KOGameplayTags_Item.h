#pragma once

#include "NativeGameplayTags.h"


namespace KOGameplayTags
{
	// ─── Factory Category ─────────────────────────────────────────────────────
	/** 보일러 (Producer) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Factory_Boiler);
	/** 모듈 분해기 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Factory_ModuleDismantler);
	/** 합금 제련기 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Factory_AlloyMaker);
	/** 기어 프레스 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Factory_GearPress);
	/** 파이프 공방 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Factory_PipeWorkshop);
	/** 지하 채굴 모듈 (Producer) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Factory_UndergroundMiningModule);
	/** 무기 작업대 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Factory_WeaponWorkbench);
	/** 직선 컨베이어 벨트 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Factory_StraightBelt);
	/** 코너(ㄱ자) 컨베이어 벨트 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Factory_CornerBelt);
	/** 압력 파이프 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Factory_PressurePipe);

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
	/** 에너지 자원 카테고리 (Producer 연료 후보 — Coal 등) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Category_EnergyResource);

	// ─── Item Identity (FKOItemRow::ItemTag) ──────────────────────────────────
	/** 기초 모듈 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_BasicModule);
	/** 석탄 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Coal);
	/** 석탄가루 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_CoalDust);
	/** 구리 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Copper);
	/** 주석 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Tin);
	/** 청동 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Bronze);
	/** 청동판 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_BronzePlate);
	/** 파손된 채광 파이프 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_DamagedMiningPipe);
	/** 채광 파이프 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_MiningPipe);
	/** 기어 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_Gear);
	/** 청동검 */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Item_BronzeSword);
}
