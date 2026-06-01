// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplayTagContainer.h"
#include "KOUISubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogKOUI, Log, All);

class UCommonActivatableWidget;
class UCommonActivatableWidgetContainerBase;
class APlayerController;
class ULocalPlayer;

/**
 * KOUISubsystem
 * LocalPlayer 수명 UI 레이어 관리 서브시스템. 프로젝트의 모든 UI 위젯은 이 서브시스템을 통해서만
 * 생성/제거된다.
 *
 * ─── 호출 일원화 (직접 호출) ───────────────────────────────────────────────
 *   게임플레이/UI 코드는 정적 헬퍼로 서브시스템을 해석해 곧바로 Open/Close 한다.
 *     UKOUISubsystem::OpenWidget(this, KOGameplayTags::UI_Widget_Inventory);
 *     UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_Inventory);
 *   (Open/Close 는 GMS 를 경유하지 않는다. World 컨텍스트로 서브시스템을 직접 잡아 호출한다.)
 *
 * ─── 루트 레이아웃 (전역 관리) ─────────────────────────────────────────────
 *   컨트롤러는 위젯을 직접 들지 않는다. SetRootLayout(UI.Layout.*) 한 줄만 호출하면,
 *   UKOUISettings::RootLayoutMap 에서 클래스를 해석해 서브시스템이 생성·소유한다.
 *
 * ─── 닫기 (Back) ───────────────────────────────────────────────────────────
 *   토글 개념은 없다. 열기는 OpenWidget, 닫기는 CommonUI Back 액션(스택 최상위
 *   위젯의 bIsBackHandler) 또는 CloseWidget(특정 위젯 지정)으로만 처리한다.
 *
 * 레이어 태그 (KOGameplayTags):
 *   UI.Layer.Game       - HUD / 게임 플레이 UI
 *   UI.Layer.GameMenu   - 게임 중 메뉴
 *   UI.Layer.Menu       - 메인 메뉴
 *   UI.Layer.Modal      - 모달 다이얼로그 (최상위)
 */
UCLASS()
class KARON_API UKOUISubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    virtual void Deinitialize() override;
    
    static UKOUISubsystem* Get(const UObject* WorldContextObject);

    /** World 컨텍스트로 서브시스템을 해석해 곧바로 Open/Close 하는 정적 헬퍼. */
    static UCommonActivatableWidget* OpenWidget(const UObject* WorldContextObject, FGameplayTag WidgetTag);
    static void CloseWidget(const UObject* WorldContextObject, FGameplayTag WidgetTag);

    // ─── Root Layout ──────────────────────────────────────────────────────────
    /** UKOUISettings::RootLayoutMap[LayoutTag] */
    void SetRootLayout(FGameplayTag LayoutTag);
    void ClearRootLayout();
    
    void RegisterPrimaryLayout(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* LayerContainer);

    // ─── Widget Stack API ─────────────────────────────────────────────────────
    /** UKOUISettings::WidgetMap */
    UCommonActivatableWidget* OpenWidget(FGameplayTag WidgetTag);
    void CloseWidget(FGameplayTag WidgetTag);
    UCommonActivatableWidget* FindActiveWidget(FGameplayTag WidgetTag) const;

private:
    UCommonActivatableWidget* PushToLayer(FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);

private:
    /** 서브시스템이 소유하는 루트 레이아웃 인스턴스 (SetRootLayout 으로 생성). */
    UPROPERTY(Transient)
    TObjectPtr<UCommonActivatableWidget> RootLayoutInstance;

    /** 등록된 Layer 컨테이너 (RegisterPrimaryLayout 으로 채움). */
    UPROPERTY(Transient)
    TMap<FGameplayTag, TObjectPtr<UCommonActivatableWidgetContainerBase>> Layers;

    /** 위젯 태그 → 로드된 클래스 캐시 (Soft 로드 결과 보관). */
    UPROPERTY(Transient)
    TMap<FGameplayTag, TSubclassOf<UCommonActivatableWidget>> ResolvedClassCache;

    /** 위젯 태그 → 현재 활성 인스턴스. Deactivate 시 자동 제거. */
    TMap<FGameplayTag, TWeakObjectPtr<UCommonActivatableWidget>> ActiveWidgetsByTag;
};
