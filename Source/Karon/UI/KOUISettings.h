// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "KOUISettings.generated.h"

class UCommonActivatableWidget;

/**
 * 위젯 식별 태그 → (대상 Layer, 위젯 클래스) 매핑 엔트리.
 */
USTRUCT()
struct FKOUIWidgetEntry
{
    GENERATED_BODY()

    /** 어느 Layer 컨테이너에 Push 할지 (UI.Layer.*). */
    UPROPERTY(EditAnywhere, Config, meta = (Categories = "UI.Layer"))
    FGameplayTag LayerTag;

    /** 실제 위젯 클래스. Soft 참조로 두어 사용 시점에 지연 로드. */
    UPROPERTY(EditAnywhere, Config)
    TSoftClassPtr<UCommonActivatableWidget> WidgetClass;
};

/**
 * KO UI 설정.
 * Project Settings → Karon → KO UI 에서 편집되며 DefaultGame.ini 에 저장된다.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "KO UI"))
class KARON_API UKOUISettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    virtual FName GetCategoryName() const override { return TEXT("Karon"); }

    static const UKOUISettings* Get() { return GetDefault<UKOUISettings>(); }

    /**
     * 루트 레이아웃 매핑.
     * Key: 컨텍스트 태그 (UI.Layout.*, 예: UI.Layout.InGame / UI.Layout.Title).
     * Value: 해당 컨텍스트에서 생성할 루트 레이아웃 위젯 클래스 (Soft 참조, 지연 로드).
     * UKOUISubsystem::SetRootLayout(Tag) 가 이 맵을 참조해 루트 레이아웃을 생성·소유한다.
     */
    UPROPERTY(EditAnywhere, Config, Category = "Root Layout",
        meta = (Categories = "UI.Layout", ForceInlineRow))
    TMap<FGameplayTag, TSoftClassPtr<UCommonActivatableWidget>> RootLayoutMap;

    /** Key: 위젯 식별 태그 (UI.Widget.*). */
    UPROPERTY(EditAnywhere, Config, Category = "Widgets",
        meta = (Categories = "UI.Widget", ForceInlineRow))
    TMap<FGameplayTag, FKOUIWidgetEntry> WidgetMap;
};
