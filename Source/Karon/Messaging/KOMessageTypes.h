// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
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
 * UI 레이어 Push 요청 메시지
 * 채널: KOGameplayTags::Data_Message_UI_PushLayerRequest ("Data.Message.UI.PushLayerRequest")
 * Layer / WidgetClass 매핑은 UKOUISettings::WidgetMap 에서 해석되므로 태그만 전달한다.
 */
USTRUCT()
struct FKOUIPushLayerRequest
{
    GENERATED_BODY()

    /** 위젯 식별 태그 (UI.Widget.*). UKOUISettings 매핑 키로 사용. */
    UPROPERTY()
    FGameplayTag WidgetTag;
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
