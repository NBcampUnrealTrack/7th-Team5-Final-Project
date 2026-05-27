#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace KOGameplayTags
{
	// ───────────────────────── Input ──────────────────────────────
	// ── Input | Native ────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Move);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Look);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Interact);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_ToggleBuildMode);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Build_Confirm);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Native_Build_ToggleDestroy);
	
	// ── Input | Ability | Movement ────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Movement_Jump);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Movement_Sprint);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Movement_Roll);

    // ── Input | Ability | Attack ──────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Combo);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Heavy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Charge);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Dash);

    // ── Input | Ability | Interaction ─────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Interaction);
	
	// ── Input | Ability | LockOn ─────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_LockOn);
    // ── Input | Ability | CoreSkill ───────────────────────────────
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_CoreSkill_);

	
	// ───────────────────────── State ──────────────────────────────
    // ── State | Character | Movement ──────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Movement_InAir);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Movement_Sprinting);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Movement_Rolling);

    // ── State | Character | Attacking ─────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking);
	//TODO:
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking_Combo);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking_Heavy);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking_Charge);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking_Dash);

    // ── State | Character | Etc ───────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Dead);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Stunned);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Invincible);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_NoStaminaRegen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_LockOn);
	
	// ───────────────────────── Event ──────────────────────────────
    // ── Event | HitReact ──────────────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact);

    // ── Event | Death ─────────────────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Death);

    // ── Event | Combo ─────────────────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combo_Window_Open);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combo_Window_Close);

	
	// ───────────────────────── Effect ─────────────────────────────
    // ── Effect | Damage ───────────────────────────────────────────
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Damage);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Heal);

    // ── Effect | Buff ─────────────────────────────────────────────
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Buff_MoveSpeed);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Buff_AttackPower);

    // ── Effect | Debuff ───────────────────────────────────────────
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Debuff_Slow);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Debuff_Stun);

	// ───────────────────────── Data ───────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Health);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Stamina);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement_WalkSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Combat); 
	
	
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
	
	// ─── Data / Message Channels ──────────────────────────────────────────────
	/** 인벤토리 변경 메시지 채널 (FKOInventoryChangedMessage 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Inventory_Changed);
	/** 팩토리 상태 변경 메시지 채널 (FKOFactoryStateChangedMessage 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Factory_StateChanged);
	/** UI 레이어 Push 요청 메시지 채널 (FKOUIPushLayerRequest 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_UI_PushLayerRequest);
	/** 건물 상호작용 메시지 채널 (FKOBuildingInteractedMessage 페이로드) */
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Message_Building_Interacted);

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

	// ─── SetbyCaller ──────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Damage);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_CoolTime);
	
	// ─── Enemy Event ──────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_SkillHit);
	
	// ─── Enemy Attack ──────────────────────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Enemy_Attack_Normal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Enemy_Attack_FirstSkill);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Enemy_Hit_Normal);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Enemy_Dead);

	// ── Input | Ability | Movement ────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Movement_Jump);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Movement_Sprint);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Movement_Roll);

    // ── Input | Ability | Attack ──────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Combo);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Heavy);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Charge);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Attack_Dash);

    // ── Input | Ability | Interaction ─────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_Interaction);
	
	// ── Input | Ability | LockOn ─────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_LockOn);
    // ── Input | Ability | CoreSkill ───────────────────────────────
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Input_Ability_CoreSkill_);

	
	// ───────────────────────── State ──────────────────────────────
    // ── State | Character | Movement ──────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Movement_InAir);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Movement_Sprinting);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Movement_Rolling);

    // ── State | Character | Attacking ─────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking);
	//TODO:
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking_Combo);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking_Heavy);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking_Charge);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Attacking_Dash);

    // ── State | Character | Etc ───────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Dead);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Stunned);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_Invincible);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_NoStaminaRegen);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Character_LockOn);
	
	// ───────────────────────── Event ──────────────────────────────
    // ── Event | HitReact ──────────────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_HitReact);

    // ── Event | Death ─────────────────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Death);

    // ── Event | Combo ─────────────────────────────────────────────
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combo_Window_Open);
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Combo_Window_Close);

	
	// ───────────────────────── Effect ─────────────────────────────
    // ── Effect | Damage ───────────────────────────────────────────
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Damage);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Heal);

    // ── Effect | Buff ─────────────────────────────────────────────
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Buff_MoveSpeed);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Buff_AttackPower);

    // ── Effect | Debuff ───────────────────────────────────────────
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Debuff_Slow);
    // UE_DECLARE_GAMEPLAY_TAG_EXTERN(Effect_Debuff_Stun);

	// ───────────────────────── Data ───────────────────────────────
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Health);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Stamina);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Movement_WalkSpeed);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Data_Attribute_Combat); 
	
	//Enemy Boss
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Boss01_Normal01);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Boss01_Normal02);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Attack_Boss01_Strong01);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Boss01_Shockwave);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Attacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Groggy);
	
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Boss01_Normal01)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Boss01_Normal02)
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Cooldown_Boss01_Strong01)
}
