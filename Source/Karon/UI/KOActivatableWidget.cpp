// Copyright Karon Team 5. All Rights Reserved.
#include "KOActivatableWidget.h"

void UKOActivatableWidget::NativeOnActivated()
{
    Super::NativeOnActivated();
}

void UKOActivatableWidget::NativeOnDeactivated()
{
    Super::NativeOnDeactivated();
}

TOptional<FUIInputConfig> UKOActivatableWidget::GetDesiredInputConfig() const
{
    switch (InputMode)
    {
    case EKOUIInputMode::Menu:
        // UI 전용: 커서 보임, 캡처 안 함.
        return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture, EMouseLockMode::DoNotLock, /*bHideCursorDuringViewportCapture=*/false);

    case EKOUIInputMode::Game:
        // 게임 전용: 커서 숨김. CapturePermanently 계열일 때만 실제 숨김 동작 (다른 캡처 모드는 CommonUI가 강제로 커서 표시함).
        return FUIInputConfig(ECommonInputMode::Game, MouseCaptureMode, EMouseLockMode::DoNotLock, /*bHideCursorDuringViewportCapture=*/true);

    case EKOUIInputMode::All:
        // 게임+UI: 커서 보임 (UI 클릭 가능).
        return FUIInputConfig(ECommonInputMode::All, MouseCaptureMode, EMouseLockMode::DoNotLock, /*bHideCursorDuringViewportCapture=*/false);

    case EKOUIInputMode::Default:
    default:
        return TOptional<FUIInputConfig>();
    }
}
