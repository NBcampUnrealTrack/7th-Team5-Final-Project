// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "GMRouterSubsystem.h"
#include "KOUISubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogKOUI, Log, All);

class UCommonActivatableWidget;
class UCommonActivatableWidgetContainerBase;
class APlayerController;
class ULocalPlayer;

/**
 * KOUISubsystem
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
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    UFUNCTION(BlueprintPure, Category = "KO|UI")
    static UKOUISubsystem* Get(const APlayerController* PlayerController);

    static UKOUISubsystem* GetForLocalPlayer(const ULocalPlayer* LocalPlayer);
    
    // Layout Registration : PrimaryLayout(HUD)안에 있는 각 레이어를 등록한다
    UFUNCTION(BlueprintCallable, Category = "KO|UI")
    void RegisterPrimaryLayout(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerContainer);

    // ─── Widget Stack API ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "KO|UI", meta = (DeterminesOutputType = "WidgetClass"))
    UCommonActivatableWidget* PushLayer(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);

    /**
     * 위젯 식별 태그(UI.Widget.*) 하나로 Push.
     * Layer / 위젯 클래스는 UKOUISettings::WidgetMap 에서 자동 해석된다.
     */
    UCommonActivatableWidget* PushWidget(FGameplayTag WidgetTag);

    UFUNCTION(BlueprintCallable, Category = "KO|UI")
    void PopLayer(UCommonActivatableWidget* Widget);

    /** 현재 열려있는(활성/스택에 존재) 위젯 인스턴스를 태그로 조회. 없으면 nullptr. */
    UFUNCTION(BlueprintPure, Category = "KO|UI")
    UCommonActivatableWidget* FindActiveWidget(FGameplayTag WidgetTag) const;

    /**
     * 위젯 토글. 열려있으면 닫고, 없으면 PushWidget.
     * @return 호출 후 위젯이 열린 상태면 true, 닫힌 상태면 false.
     */
    UFUNCTION(BlueprintCallable, Category = "KO|UI")
    bool ToggleWidget(FGameplayTag WidgetTag);

private:
    UFUNCTION()
    void OnPushLayerRequestReceived(FGameplayTag Channel, const FInstancedStruct& Payload);

    /** 등록된 Layer 컨테이너 (RegisterPrimaryLayout 으로 채움). */
    UPROPERTY(Transient)
    TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;

    /** 위젯 태그 → 로드된 클래스 캐시 (Soft 로드 결과 보관). */
    UPROPERTY(Transient)
    TMap<FGameplayTag, TSubclassOf<UCommonActivatableWidget>> ResolvedClassCache;

    /** 위젯 태그 → 현재 활성 인스턴스. Deactivate 시 자동 제거. */
    TMap<FGameplayTag, TWeakObjectPtr<UCommonActivatableWidget>> ActiveWidgetsByTag;

    FGameplayMessageCallback PushLayerCallback;
    FGameplayMessageHandle PushLayerHandle;
};
