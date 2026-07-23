// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Component/Build/KOGridBuildComponent.h"
#include "Data/Type/KOSkillTypes.h"
#include "KOMessageTypes.generated.h"

/**
 * 인벤토리 변경 메시지
 * 채널: KOGameplayTags::Data_Message_Inventory_Changed ("Data.Message.Inventory.Changed")
 */
USTRUCT()
struct FKOInventoryChangedMessage
{
    GENERATED_BODY()

    UPROPERTY()
    FName ItemId;

    UPROPERTY()
    int32 NewCount = 0;

    UPROPERTY()
    int32 PreviousCount = 0;
};

/**
 * 팩토리 상태 변경 메시지
 * 채널: KOGameplayTags::Data_Message_Factory_StateChanged ("Data.Message.Factory.StateChanged")
 */
USTRUCT()
struct FKOFactoryStateChangedMessage
{
    GENERATED_BODY()

    UPROPERTY()
    FName FactoryId;

    UPROPERTY()
    bool bIsActive = false;

    UPROPERTY()
    float Progress = 0.f;
};

/**
 * 건물 상호작용 메시지
 * 채널: KOGameplayTags::Data_Message_Building_Interacted ("Data.Message.Building.Interacted")
 */
USTRUCT()
struct FKOBuildingInteractedMessage
{
    GENERATED_BODY()

    UPROPERTY()
    FName FactoryId;

    UPROPERTY()
    TWeakObjectPtr<AActor> Building;

    UPROPERTY()
    TWeakObjectPtr<AActor> Instigator;
};

/**
 * 건설 퀵슬롯 할당 변경 메시지
 * 채널: KOGameplayTags::Data_Message_Build_QuickSlotChanged
 */
USTRUCT()
struct FKOBuildQuickSlotChangedMessage
{
    GENERATED_BODY()

    UPROPERTY()
    int32 SlotIndex = INDEX_NONE;

    UPROPERTY()
    FName FactoryId = NAME_None;
};

/**
 * 건설 퀵슬롯 선택 변경 메시지
 * 채널: KOGameplayTags::Data_Message_Build_QuickSlotSelectionChanged
 */
USTRUCT()
struct FKOBuildQuickSlotSelectionChangedMessage
{
    GENERATED_BODY()

    UPROPERTY()
    int32 PreviousSlotIndex = INDEX_NONE;

    UPROPERTY()
    int32 NewSlotIndex = INDEX_NONE;
};

/**
 * 건설 모드 변경 메시지
 * 채널: KOGameplayTags::Data_Message_Build_ModeChanged
 */
USTRUCT()
struct FKOBuildModeChangedMessage
{
    GENERATED_BODY()

    UPROPERTY()
    EKOGridBuildMode PreviousMode = EKOGridBuildMode::None;

    UPROPERTY()
    EKOGridBuildMode NewMode = EKOGridBuildMode::None;
};

/**
 * Producer 연료 변경 메시지 (적재 / 비움 — 카운트 매 틱 변화는 브로드캐스트하지 않음)
 * 채널: KOGameplayTags::Data_Message_Producer_FuelChanged
 */
USTRUCT()
struct FKOProducerFuelChangedMessage
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<class UKOEnergyProducerComponent> Producer;

    UPROPERTY()
    FName FuelItemId = NAME_None;

    UPROPERTY()
    int32 FuelCount = 0;
};

/**
 * Processor 상태/버퍼/레시피 변경 메시지 (Progress 진행은 매 틱이라 미포함).
 * 채널: KOGameplayTags::Data_Message_Processor_Changed
 */
USTRUCT()
struct FKOProcessorChangedMessage
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<class UKOFactoryProcessorComponent> Processor;
};
/**
 * 아이템 획득
 * 채널: KOGameplayTags::Event_DropItem
 */
USTRUCT()
struct FKODropItemMessage
{
    GENERATED_BODY()
    
    UPROPERTY()
    FName    EnemyTag;
    
    UPROPERTY()
    int32    EnemyLevel = 0;
    
    UPROPERTY()
    FName ItemId;
    
    UPROPERTY()
    int32 Count=0;
};

/**
 * 상호작용
 * 채널: KOGameplayTags::Event_Interaction
 */
USTRUCT()
struct FKOInteractionMessage
{
    GENERATED_BODY()

    UPROPERTY()
    FText InteractionId;
    
    UPROPERTY()
    bool bIsActive = true;
};

/**
 * 스킬 퀵슬롯 배정 변경 메시지
 * 채널: KOGameplayTags::Data_Message_Skill_QuickSlotChanged ("Data.Message.Skill.QuickSlotChanged")
 */
USTRUCT()
struct FKOSkillQuickSlotChangedMessage
{
    GENERATED_BODY()

    UPROPERTY()
    ESkillQuickSlotKey SlotKey = ESkillQuickSlotKey::Q;

    /** 새로 배정된 스킬 이름. NAME_None이면 슬롯이 비워진 것. */
    UPROPERTY()
    FName SkillName = NAME_None;

    UPROPERTY()
    FGameplayTag SkillTag;
};

/**
 * 오버클럭 프로그레스바
 * 채널: KOGameplayTags::Event_SyncOverclockProgressBar
 */
USTRUCT()
struct FKOOverclockProgressBarMessage
{
    GENERATED_BODY()

    UPROPERTY()
    float Ratio = 0.f;
};

/**
 * Telemetry 전투 수집용 메세지
 * 채널: KOGameplayTags::Event_Telemetry_Combat
 */
USTRUCT()
struct FKOTelemetryCombatMessage
{
    GENERATED_BODY()
    
    UPROPERTY()
    FName    EnemyTag;
    
    UPROPERTY()
    int32    EnemyLevel = 0;
    
    UPROPERTY()
    FString  AbilityName;
    
    UPROPERTY()
    float	 Value = 0.f;
    
    UPROPERTY()
    float	 HealthPercentAfter=0.f;
    
    UPROPERTY()
    FVector  Position = FVector::ZeroVector;
};

/**
 * 범용 텍스트 메세지 구조체
 */
USTRUCT()
struct FKOTextMessage
{
    GENERATED_BODY()
    
    UPROPERTY()
    FText InText;
};
