// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "KOMessageTypes.generated.h"

/**
 * 인벤토리 변경 메시지
 * 채널: KOGameplayTags::Message_Inventory_Changed ("Message.Inventory.Changed")
 *
 * 사용 예:
 *   FKOInventoryChangedMessage Msg;
 *   Msg.ItemTag = SomeTag;
 *   Msg.NewCount = 5;
 *   MessageSubsystem->BroadcastMessage(KOGameplayTags::Message_Inventory_Changed, FInstancedStruct::Make(Msg));
 */
USTRUCT(BlueprintType)
struct FKOInventoryChangedMessage
{
    GENERATED_BODY()

    /** 변경된 아이템을 식별하는 GameplayTag */
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    FGameplayTag ItemTag;

    /** 변경 후 수량 */
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    int32 NewCount = 0;

    /** 변경 전 수량 */
    UPROPERTY(BlueprintReadWrite, Category = "Inventory")
    int32 PreviousCount = 0;
};

/**
 * 팩토리 상태 변경 메시지
 * 채널: KOGameplayTags::Message_Factory_StateChanged ("Message.Factory.StateChanged")
 *
 * 사용 예:
 *   FKOFactoryStateChangedMessage Msg;
 *   Msg.FactoryTag = SomeFactoryTag;
 *   Msg.bIsActive = true;
 *   MessageSubsystem->BroadcastMessage(KOGameplayTags::Message_Factory_StateChanged, FInstancedStruct::Make(Msg));
 */
USTRUCT(BlueprintType)
struct FKOFactoryStateChangedMessage
{
    GENERATED_BODY()

    /** 팩토리를 식별하는 GameplayTag */
    UPROPERTY(BlueprintReadWrite, Category = "Factory")
    FGameplayTag FactoryTag;

    /** 팩토리 활성화 여부 */
    UPROPERTY(BlueprintReadWrite, Category = "Factory")
    bool bIsActive = false;

    /** 현재 생산 진행률 (0.0 ~ 1.0) */
    UPROPERTY(BlueprintReadWrite, Category = "Factory")
    float Progress = 0.f;
};

/**
 * UI 레이어 Push 요청 메시지
 * 채널: KOGameplayTags::Message_UI_PushLayerRequest ("Message.UI.PushLayerRequest")
 *
 * 사용 예:
 *   FKOUIPushLayerRequest Msg;
 *   Msg.LayerTag  = KOGameplayTags::UI_Layer_Menu;
 *   Msg.WidgetClass = UMyMenuWidget::StaticClass();
 *   MessageSubsystem->BroadcastMessage(KOGameplayTags::Message_UI_PushLayerRequest, FInstancedStruct::Make(Msg));
 */
USTRUCT(BlueprintType)
struct FKOUIPushLayerRequest
{
    GENERATED_BODY()

    /** 대상 레이어 태그 (예: UI.Layer.Menu) */
    UPROPERTY(BlueprintReadWrite, Category = "UI")
    FGameplayTag LayerTag;

    /** Push 할 위젯 클래스 */
    UPROPERTY(BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UCommonActivatableWidget> WidgetClass;
};