// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "KHS_GMRouterManager.h"
#include "KOUISubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogKOUI, Log, All);

class UCommonActivatableWidget;
class UCommonActivatableWidgetContainerBase;

/**
 * KOUISubsystem
 *
 * LocalPlayer 수명 UI 레이어 관리 서브시스템.
 * HUD(또는 기타 GameFramework 클래스)에서 RegisterPrimaryLayout()을 통해
 * 레이어 컨테이너 위젯을 등록하면, 이후 모든 위젯 Push/Pop은 이 서브시스템을 통한다.
 *
 * 메시지 채널:
 *   - Message.UI.PushLayerRequest (FKOUIPushLayerRequest) 를 구독.
 *     외부 시스템이 메시지로 레이어 Push를 요청할 수 있다.
 *
 * 레이어 태그 (KOGameplayTags):
 *   UI.Layer.Game       - HUD / 게임 플레이 UI
 *   UI.Layer.GameMenu   - 게임 중 메뉴 (일시정지 등)
 *   UI.Layer.Menu       - 메인 메뉴
 *   UI.Layer.Modal      - 모달 다이얼로그 (최상위)
 */
UCLASS()
class KARON_API UKOUISubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    // ─── USubsystem ──────────────────────────────────────────────────────────
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ─── Layout Registration ──────────────────────────────────────────────────
    /**
     * HUD 또는 전용 위젯이 생성된 레이어 컨테이너를 이 서브시스템에 등록.
     * 동일 태그로 다시 호출하면 기존 등록을 덮어쓴다.
     *
     * @param LayerTag       레이어를 식별하는 태그 (예: UI.Layer.Menu)
     * @param LayerContainer 해당 레이어에 쓸 UCommonActivatableWidgetContainerBase 인스턴스
     */
    UFUNCTION(BlueprintCallable, Category = "KO|UI")
    void RegisterPrimaryLayout(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerContainer);

    // ─── Widget Stack API ─────────────────────────────────────────────────────
    /**
     * 지정한 레이어에 위젯 클래스를 Push한다.
     * 해당 레이어가 등록되어 있지 않으면 nullptr를 반환하고 경고를 출력한다.
     *
     * @param LayerTag    대상 레이어 태그
     * @param WidgetClass Push할 UCommonActivatableWidget 파생 클래스
     * @return            생성/풀에서 반환된 위젯 인스턴스 (소유권은 컨테이너)
     */
    UFUNCTION(BlueprintCallable, Category = "KO|UI", meta = (DeterminesOutputType = "WidgetClass"))
    UCommonActivatableWidget* PushLayer(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);

    /**
     * 위젯 인스턴스를 속해 있는 레이어에서 제거한다.
     * 컨테이너가 스택이면 해당 위젯이 Pop된다.
     *
     * @param Widget 제거할 위젯 인스턴스
     */
    UFUNCTION(BlueprintCallable, Category = "KO|UI")
    void PopLayer(UCommonActivatableWidget* Widget);

private:
    // ─── GMS 콜백 ─────────────────────────────────────────────────────────────
    /**
     * Message.UI.PushLayerRequest 채널 수신 콜백.
     * FKOUIPushLayerRequest 페이로드를 파싱해 PushLayer()를 호출한다.
     */
    UFUNCTION()
    void OnPushLayerRequestReceived(FGameplayTag Channel, const FInstancedStruct& Payload);

    // ─── 내부 상태 ────────────────────────────────────────────────────────────
    /** 레이어 태그 → 컨테이너 위젯 맵. RegisterPrimaryLayout()으로 채운다. */
    UPROPERTY(Transient)
    TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;

    /** GMS Unsubscribe에 필요한 콜백 레퍼런스 */
    FGameplayMessageCallback PushLayerCallback;

    /** GMS 구독 채널 태그 (Deinitialize에서 Unsubscribe할 때 사용) */
    FGameplayTag PushLayerChannel;
};
