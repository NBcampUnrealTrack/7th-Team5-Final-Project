// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "CommonActivatableWidget.h"
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
 */
USTRUCT()
struct FKOUIPushLayerRequest
{
    GENERATED_BODY()

    /** 대상 레이어 태그 (예: UI.Layer.Menu) */
    UPROPERTY()
    FGameplayTag LayerTag;

    /** Push 할 위젯 클래스 */
    UPROPERTY()
    TSubclassOf<UCommonActivatableWidget> WidgetClass;
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
