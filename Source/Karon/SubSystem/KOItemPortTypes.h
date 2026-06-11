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

/** 머신 포트 방향(입력/출력). 벨트 연결 UI/바인딩 공용. */
UENUM(BlueprintType)
enum class EKOPortKind : uint8
{
    Input,
    Output
};

/**
 * 공장 포트 슬롯 한 칸(일반 포트 모델).
 * 슬롯 = (Kind, PortIndex) — 머신의 입력/출력 포트 위치이며, 이것이 벨트 바인딩의 키다.
 * ItemId 는 표시용 힌트(선택 레시피의 해당 포트 아이템)일 뿐 바인딩 키가 아니다.
 * 레시피 미선택/빈 포트면 ItemId=None 이어도 포트 자체는 유효한 바인딩 대상이다.
 */
USTRUCT(BlueprintType)
struct FKOFactoryPortSlot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "KO|Port")
    EKOPortKind Kind = EKOPortKind::Input;

    /** 그룹(Input/Output) 내 포트 위치. 벨트 바인딩의 키. */
    UPROPERTY(BlueprintReadOnly, Category = "KO|Port")
    int32 PortIndex = 0;

    /** 표시용 아이템 힌트(선택 레시피의 해당 포트 아이템). 없으면 빈 포트. 바인딩 키 아님. */
    UPROPERTY(BlueprintReadOnly, Category = "KO|Port")
    FName ItemId = NAME_None;

    FKOFactoryPortSlot() = default;
    FKOFactoryPortSlot(EKOPortKind InKind, int32 InPortIndex, FName InItemId = NAME_None)
        : Kind(InKind), PortIndex(InPortIndex), ItemId(InItemId) {}

    /** 포트는 빈 칸이어도 유효한 바인딩 대상이므로 PortIndex 기준. */
    bool IsValid() const { return PortIndex >= 0; }
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