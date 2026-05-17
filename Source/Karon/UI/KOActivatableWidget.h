// Copyright Karon Team 5. All Rights Reserved.
#pragma once

/**
 * ============================================================
 *  KHS_GameplayMessageRouter (GMS) 사용 가이드
 * ============================================================
 *
 * [서브시스템 접근]
 *   UKHS_GMRouterManager* GMS = GetGameInstance()->GetSubsystem<UKHS_GMRouterManager>();
 *
 * ─── 1. Broadcast (메시지 송신) ────────────────────────────
 *   // ① 페이로드 구조체 작성 (KOMessageTypes.h 참고)
 *   FKOInventoryChangedMessage Msg;
 *   Msg.ItemTag      = KOGameplayTags::SomeItemTag;
 *   Msg.NewCount     = 5;
 *   Msg.PreviousCount = 3;
 *
 *   // ② FInstancedStruct::Make로 박싱 후 전송
 *   if (UKHS_GMRouterManager* GMS = GetGameInstance()->GetSubsystem<UKHS_GMRouterManager>())
 *   {
 *       GMS->BroadcastMessage(KOGameplayTags::Message_Inventory_Changed,
 *                             FInstancedStruct::Make(Msg));
 *   }
 *
 * ─── 2. Subscribe (메시지 구독) ────────────────────────────
 *   // ① 콜백 멤버 변수 선언 (헤더에 UPROPERTY 없이 멤버로 보관)
 *   //   FGameplayMessageCallback InventoryCallback;
 *
 *   // ② BindDynamic으로 바인딩
 *   InventoryCallback.BindDynamic(this, &UMyWidget::OnInventoryChanged);
 *
 *   // ③ 구독 등록
 *   if (UKHS_GMRouterManager* GMS = GetGameInstance()->GetSubsystem<UKHS_GMRouterManager>())
 *   {
 *       GMS->SubscribeToMessage(KOGameplayTags::Message_Inventory_Changed, InventoryCallback);
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
 *   // NativeOnDeactivated 또는 NativeDestruct에서 반드시 해제
 *   if (UKHS_GMRouterManager* GMS = GetGameInstance()->GetSubsystem<UKHS_GMRouterManager>())
 *   {
 *       GMS->Unsubscribe(KOGameplayTags::Message_Inventory_Changed, InventoryCallback);
 *   }
 *   InventoryCallback.Clear();
 *
 * ─── 5. KOActivatableWidget 상속 시 권장 패턴 ──────────────
 *   - 구독 → NativeOnActivated()  오버라이드 후 Super:: 호출 전에 등록
 *   - 해제 → NativeOnDeactivated() 오버라이드 후 Super:: 호출 전에 해제
 *   - 이렇게 하면 위젯이 화면에 보일 때만 메시지를 수신한다.
 *
 * ─── 6. FKOUIPushLayerRequest 예시 ────────────────────────
 *   // 다른 시스템에서 UI 레이어 Push를 요청하는 방법
 *   FKOUIPushLayerRequest Req;
 *   Req.LayerTag   = KOGameplayTags::UI_Layer_Menu;
 *   Req.WidgetClass = UMyMenuWidget::StaticClass();
 *   GMS->BroadcastMessage(KOGameplayTags::Message_UI_PushLayerRequest,
 *                         FInstancedStruct::Make(Req));
 *
 * ─── 주의사항 ───────────────────────────────────────────────
 *   - FGameplayMessageCallback은 UFUNCTION()이 붙은 함수만 바인딩 가능
 *   - 구독 해제 없이 위젯이 파괴되면 댕글링 델리게이트 발생 가능
 *   - 페이로드 구조체는 KOMessageTypes.h에 정의하는 것을 권장 (팀 컨벤션)
 * ============================================================
 */

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "KHS_GMRouterManager.h"
#include "KOActivatableWidget.generated.h"

/**
 * KOActivatableWidget
 *
 * 프로젝트 전용 CommonActivatableWidget 베이스.
 * KHS GMS 구독/해제를 NativeOnActivated/NativeOnDeactivated와 연동해주는
 * 헬퍼 패턴을 제공한다.
 *
 * 사용법:
 *   1. 이 클래스를 상속받아 파생 위젯 생성
 *   2. NativeOnActivated()에서 Super::NativeOnActivated() 호출 후 GMS 구독
 *   3. NativeOnDeactivated()에서 GMS 해제 후 Super::NativeOnDeactivated() 호출
 *   4. 수신 함수는 반드시 UFUNCTION() 마크 필요
 *
 * 예시 파생 클래스: UKOInventoryWidget, UKOFactoryWidget
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class KARON_API UKOActivatableWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

protected:
    // ─── UCommonActivatableWidget ─────────────────────────────────────────────
    /**
     * 위젯이 활성화될 때 호출.
     * 파생 클래스에서 오버라이드 시 Super::NativeOnActivated() 반드시 호출.
     * GMS 구독은 이 함수 내 Super 호출 이후에 등록한다.
     */
    virtual void NativeOnActivated() override;

    /**
     * 위젯이 비활성화될 때 호출.
     * 파생 클래스에서 오버라이드 시 Super::NativeOnDeactivated() 반드시 호출.
     * GMS 구독 해제는 이 함수 내 Super 호출 이전에 수행한다.
     */
    virtual void NativeOnDeactivated() override;

    // ─── GMS 헬퍼 ────────────────────────────────────────────────────────────
    /**
     * GMS 채널 구독 헬퍼.
     * NativeOnActivated 내에서 호출하면 위젯이 활성 상태일 때만 메시지를 수신한다.
     *
     * @param Channel  구독할 GameplayTag 채널
     * @param Callback UFUNCTION()이 붙은 수신 델리게이트
     */
    UFUNCTION(BlueprintCallable, Category = "KO|UI|GMS", meta = (BlueprintProtected = "true"))
    void SubscribeToGMS(FGameplayTag Channel, const FGameplayMessageCallback& Callback);

    /**
     * GMS 채널 구독 해제 헬퍼.
     * NativeOnDeactivated 내에서 호출한다.
     *
     * @param Channel  해제할 GameplayTag 채널
     * @param Callback 등록 시 사용한 동일한 델리게이트
     */
    UFUNCTION(BlueprintCallable, Category = "KO|UI|GMS", meta = (BlueprintProtected = "true"))
    void UnsubscribeFromGMS(FGameplayTag Channel, const FGameplayMessageCallback& Callback);

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
    UKHS_GMRouterManager* GetGMSSubsystem() const;
};
