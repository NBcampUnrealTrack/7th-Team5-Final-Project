// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "KOItemPortTypes.generated.h"

/**
 * 컨베이어 위를 흐르는 아이템 한 단위.
 * 현재는 ItemId(인벤토리와 동일한 FName 식별자)만 담는다.
 * per-item 가변 데이터(품질/내구도 등)가 생기면 이 구조체에 필드를 추가하면 되며,
 * 포트 인터페이스 시그니처는 바꿀 필요가 없다(확장점).
 */
USTRUCT()
struct FKOConveyorItem
{
    GENERATED_BODY()

    UPROPERTY()
    FName ItemId = NAME_None;

    FKOConveyorItem() = default;
    explicit FKOConveyorItem(FName InItemId) : ItemId(InItemId) {}

    bool IsValid() const { return !ItemId.IsNone(); }
    void Reset()         { ItemId = NAME_None; }
};

// ─────────────────────────────────────────────────────────────────────────────
// 아이템을 "내보내는" 쪽 (머신 출력 버퍼 / 벨트 꼬리).
// 파괴 가능한 그리드 이웃을 Cast<>/FindComponentByInterface 로 해결해야 하므로
// 순수 C++ 인터페이스가 아니라 UINTERFACE 로 둔다.
// ─────────────────────────────────────────────────────────────────────────────
UINTERFACE()
class UKOItemSource : public UInterface
{
    GENERATED_BODY()
};

class IKOItemSource
{
    GENERATED_BODY()

public:
    /** 이동 없이 다음에 내보낼 아이템을 확인. 내보낼 게 없으면 false. */
    virtual bool PeekOutputItem(FKOConveyorItem& OutItem) const = 0;

    /** 아이템 1개를 실제로 내보내고 제거. 성공 시 OutItem 채우고 true. */
    virtual bool PopOutputItem(FKOConveyorItem& OutItem) = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// 아이템을 "받아들이는" 쪽 (머신 입력 / 연료 입구 / 벨트 머리).
// ─────────────────────────────────────────────────────────────────────────────
UINTERFACE()
class UKOItemSink : public UInterface
{
    GENERATED_BODY()
};

class IKOItemSink
{
    GENERATED_BODY()

public:
    /** 이 아이템을 받을 수 있는지(이동 없이 검사). */
    virtual bool CanAcceptItem(const FKOConveyorItem& Item) const = 0;

    /** 아이템 1개를 실제로 받아들임. 성공 시 true. */
    virtual bool PushItem(const FKOConveyorItem& Item) = 0;
};