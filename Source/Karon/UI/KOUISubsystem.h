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
  
    UFUNCTION(BlueprintCallable, Category = "KO|UI")
    void PopLayer(UCommonActivatableWidget* Widget);

private:
    UFUNCTION()
    void OnPushLayerRequestReceived(FGameplayTag Channel, const FInstancedStruct& Payload);

    UPROPERTY(Transient)
    TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;

    FGameplayMessageCallback PushLayerCallback;
    FGameplayMessageHandle PushLayerHandle;
};
