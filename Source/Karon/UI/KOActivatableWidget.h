// Copyright Karon Team 5. All Rights Reserved.
#pragma once

/**
 * ============================================================
 *  GMRouter (GMS) 사용 가이드
 * ============================================================
 *
 * GMS 헬퍼는 IKOGMSInterface(Messaging/KOGMSInterface.h)에 정의되어 있으며,
 * UKOActivatableWidget은 이 인터페이스를 상속하여 동일 API를 그대로 사용한다.
 *
 * ─── 1. Broadcast (메시지 송신) ────────────────────────────
 *   FKOInventoryChangedMessage Msg;
 *   Msg.ItemId = TEXT("Item_Iron"); Msg.NewCount = 5; Msg.PreviousCount = 3;
 *   Broadcast(KOGameplayTags::Data_Message_Inventory_Changed,
 *             FInstancedStruct::Make(Msg));
 *
 * ─── 2. Subscribe (메시지 구독) ────────────────────────────
 *   // 멤버: FGameplayMessageCallback Cb; FGameplayMessageHandle H;
 *   Cb.BindDynamic(this, &UMyWidget::OnInventoryChanged);
 *   H = Subscribe(KOGameplayTags::Data_Message_Inventory_Changed, Cb);
 *
 * ─── 3. 수신 함수 시그니처 ─────────────────────────────────
 *   UFUNCTION()
 *   void OnInventoryChanged(FGameplayTag Channel, const FInstancedStruct& Payload);
 *
 * ─── 4. Unsubscribe (구독 해제) ────────────────────────────
 *   Unsubscribe(H);
 *
 * ─── 5. KOActivatableWidget 상속 시 권장 패턴 ──────────────
 *   - NativeOnActivated   : Super:: 호출 후 Subscribe로 핸들 저장
 *   - NativeOnDeactivated : Unsubscribe(Handle) 후 Super:: 호출
 *
 */

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Messaging/KOGMSInterface.h"
#include "KOActivatableWidget.generated.h"

/**
 * KOActivatableWidget
 * 프로젝트 전용 CommonActivatableWidget 베이스.
 * GMRouter 연동은 IKOGMSInterface를 통해
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOActivatableWidget : public UCommonActivatableWidget, public IKOGMSInterface
{
    GENERATED_BODY()

protected:
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
};
