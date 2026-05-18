// Copyright Karon Team 5. All Rights Reserved.
#pragma once

/**
 * ============================================================
 *  GMRouter (GMS) 사용 가이드
 * ============================================================
 *
 * [서브시스템 접근]
 *   UGMRouterSubsystem* GMS = GetGameInstance()->GetSubsystem<UGMRouterSubsystem>();
 *
 * ─── 1. Broadcast (메시지 송신) ────────────────────────────
 *   FKOInventoryChangedMessage Msg;
 *   Msg.ItemTag       = KOGameplayTags::SomeItemTag;
 *   Msg.NewCount      = 5;
 *   Msg.PreviousCount = 3;
 *
 *   if (UGMRouterSubsystem* GMS = GetGameInstance()->GetSubsystem<UGMRouterSubsystem>())
 *   {
 *       GMS->BroadcastMessage(KOGameplayTags::Data_Message_Inventory_Changed,
 *                             FInstancedStruct::Make(Msg));
 *   }
 *
 * ─── 2. Subscribe (메시지 구독) ────────────────────────────
 *   // ① 콜백 + 핸들 멤버 변수
 *   //   FGameplayMessageCallback InventoryCallback;
 *   //   FGameplayMessageHandle   InventoryHandle;
 *
 *   // ② BindDynamic
 *   InventoryCallback.BindDynamic(this, &UMyWidget::OnInventoryChanged);
 *
 *   // ③ Subscribe → 반환 핸들 저장 (Unsubscribe에 사용)
 *   if (UGMRouterSubsystem* GMS = GetGameInstance()->GetSubsystem<UGMRouterSubsystem>())
 *   {
 *       InventoryHandle = GMS->Subscribe(KOGameplayTags::Data_Message_Inventory_Changed, InventoryCallback);
 *   }
 *
 * ─── 3. 수신 함수 시그니처 ─────────────────────────────────
 *   UFUNCTION()
 *   void OnInventoryChanged(FGameplayTag Channel, const FInstancedStruct& Payload)
 *   {
 *       if (const FKOInventoryChangedMessage* Data = Payload.GetPtr<FKOInventoryChangedMessage>())
 *       {
 *           // Data->NewCount, Data->ItemTag 등 사용
 *       }
 *   }
 *
 * ─── 4. Unsubscribe (구독 해제) ────────────────────────────
 *   // 핸들 기반으로 해제. GMRouter는 객체 소멸 시 자동 GC 정리되지만
 *   // 위젯 비활성화 단계에서 명시적 해제를 권장.
 *   if (UGMRouterSubsystem* GMS = GetGameInstance()->GetSubsystem<UGMRouterSubsystem>())
 *   {
 *       GMS->Unsubscribe(InventoryHandle);
 *   }
 *
 * ─── 5. KOActivatableWidget 상속 시 권장 패턴 ──────────────
 *   - 구독 → NativeOnActivated()에서 Super:: 호출 후 SubscribeToGMS()로 핸들 저장
 *   - 해제 → NativeOnDeactivated()에서 UnsubscribeFromGMS(Handle) 후 Super:: 호출
 *
 * ─── 6. FKOUIPushLayerRequest 예시 ────────────────────────
 *   FKOUIPushLayerRequest Req;
 *   Req.LayerTag    = KOGameplayTags::UI_Layer_Menu;
 *   Req.WidgetClass = UMyMenuWidget::StaticClass();
 *   GMS->BroadcastMessage(KOGameplayTags::Data_Message_UI_PushLayerRequest,
 *                         FInstancedStruct::Make(Req));
 *
 * ─── 주의사항 ───────────────────────────────────────────────
 *   - FGameplayMessageCallback은 UFUNCTION()이 붙은 함수만 바인딩 가능
 *   - Subscribe 반환 FGameplayMessageHandle을 보관해야 Unsubscribe 가능
 *   - 페이로드 구조체는 KOMessageTypes.h에 정의 (팀 컨벤션)
 * ============================================================
 */

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "GMRouterSubsystem.h"
#include "KOActivatableWidget.generated.h"

/**
 * KOActivatableWidget
 *
 * 프로젝트 전용 CommonActivatableWidget 베이스.
 * GMRouter GMS 구독/해제를 NativeOnActivated/NativeOnDeactivated와 연동해주는
 * 헬퍼를 제공한다.
 *
 * 파생 클래스 규칙:
 *   - NativeOnActivated()  : Super 호출 후 GMS 구독, 반환 핸들 저장
 *   - NativeOnDeactivated(): 핸들 기반 GMS 해제 후 Super 호출
 *   - 수신 함수는 반드시 UFUNCTION() 마크 필요
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOActivatableWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    // ─── UCommonActivatableWidget ─────────────────────────────────────────────
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;

    // ─── GMS 헬퍼 ────────────────────────────────────────────────────────────
    /**
     * GMS 채널 구독 헬퍼. 반환된 FGameplayMessageHandle을 보관해야 Unsubscribe 가능.
     * NativeOnActivated 내에서 호출하면 위젯이 활성 상태일 때만 메시지를 수신한다.
     *
     * @param Channel  구독할 GameplayTag 채널
     * @param Callback UFUNCTION()이 붙은 수신 델리게이트
     * @return         구독 해제용 핸들 (실패 시 IsValid()==false)
     */
    UFUNCTION(BlueprintCallable, Category = "KO|UI|GMS", meta = (BlueprintProtected = "true"))
    FGameplayMessageHandle SubscribeToGMS(FGameplayTag Channel, const FGameplayMessageCallback& Callback);

    /**
     * GMS 채널 구독 해제 헬퍼.
     * NativeOnDeactivated 내에서 호출한다.
     *
     * @param Handle SubscribeToGMS가 반환한 핸들
     */
    UFUNCTION(BlueprintCallable, Category = "KO|UI|GMS", meta = (BlueprintProtected = "true"))
    void UnsubscribeFromGMS(const FGameplayMessageHandle& Handle);

    /**
     * GMS Broadcast 헬퍼.
     * 어느 라이프사이클 단계에서도 호출 가능.
     *
     * @param Channel 브로드캐스트할 GameplayTag 채널
     * @param Payload FInstancedStruct::Make(구조체)로 생성한 페이로드
     */
    UFUNCTION(BlueprintCallable, Category = "KO|UI|GMS", meta = (BlueprintProtected = "true"))
    void BroadcastGMS(FGameplayTag Channel, const FInstancedStruct& Payload);

private:
    /** GMS 서브시스템 접근 헬퍼. GameInstance가 유효하지 않으면 nullptr 반환. */
    UGMRouterSubsystem* GetGMSSubsystem() const;
};
