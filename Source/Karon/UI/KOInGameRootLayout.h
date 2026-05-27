// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "KOActivatableWidget.h"
#include "KOInGameRootLayout.generated.h"

class UCommonActivatableWidgetContainerBase;

/**
 * UKOInGameRootLayout
 * 인게임 화면의 루트 레이아웃. NativeConstruct에서 UISubsystem에
 * Game / GameMenu / Modal 3개 레이어 컨테이너를 등록한다.
 *
 * BP(`WBP_InGameRootLayout`)에서 같은 이름으로 CommonActivatableWidgetStack을
 * 배치해야 BindWidget이 성공한다.
 */
UCLASS()
class KARON_API UKOInGameRootLayout : public UKOActivatableWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;

    /** HUD / 게임 플레이 UI (항상 표시되는 최하위 레이어). */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonActivatableWidgetContainerBase> GameLayer;

    /** 인벤토리·빌드 메뉴·공장 상호작용 패널 등 중간 레이어. 게임 시간은 흐름. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonActivatableWidgetContainerBase> GameMenuLayer;

    /** Pause / Settings / Quit-to-Title 등 게임을 멈추는 전체 메뉴 레이어. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonActivatableWidgetContainerBase> MenuLayer;

    /** 확인 다이얼로그 등 최상위 모달 레이어. */
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCommonActivatableWidgetContainerBase> ModalLayer;
};
