// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "GMRouterSubsystem.h"
#include "KOGMSInterface.generated.h"

struct FInstancedStruct;

UINTERFACE(MinimalAPI, meta = (CannotImplementInterfaceInBlueprint))
class UKOGMSInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 * IKOGMSInterface
 *
 * GMRouter(GMS)에 대한 Subscribe / Unsubscribe / Broadcast 헬퍼를 제공하는 믹스인 인터페이스.
 * 구현체(반드시 UObject 파생)는 별도의 멤버 추가 없이 GMS 채널 핸들링이 가능하다.
 *
 * 위젯 전용이 아니며 액터, 컴포넌트, 서브시스템 등 어떤 UObject에도 부착하여 사용 가능하다.
 *
 * 사용 패턴 (UKOActivatableWidget 등):
 *   NativeOnActivated:   Callback.BindDynamic(...); Handle = Subscribe(Channel, Callback);
 *   NativeOnDeactivated: Unsubscribe(Handle);
 *
 * 비고:
 *   - 메서드는 UFUNCTION이 아닌 일반 C++ virtual로 선언되어 있어 BP에서 직접 호출되지 않는다.
 *     BP에서 GMS를 호출할 필요가 있다면 별도 UBlueprintFunctionLibrary를 둘 것.
 *   - GetWorld()는 구현 객체(UObject)에서 가져오므로, WorldContext가 유효한 객체에서만 동작한다.
 */
class KARON_API IKOGMSInterface
{
    GENERATED_BODY()

public:
    /** GMS 채널 구독. 반환된 핸들을 보관해야 Unsubscribe 가능. 실패 시 IsValid()==false. */
    virtual FGameplayMessageHandle Subscribe(FGameplayTag Channel, const FGameplayMessageCallback& Callback);

    /** 핸들 기반 GMS 구독 해제. 유효하지 않은 핸들은 무시. */
    virtual void Unsubscribe(const FGameplayMessageHandle& Handle);

    /** GMS Broadcast. */
    virtual void Broadcast(FGameplayTag Channel, const FInstancedStruct& Payload);

protected:
    /** GMS 서브시스템 접근 헬퍼. WorldContext가 유효하지 않으면 nullptr. */
    virtual UGMRouterSubsystem* GetGMS() const;
};
