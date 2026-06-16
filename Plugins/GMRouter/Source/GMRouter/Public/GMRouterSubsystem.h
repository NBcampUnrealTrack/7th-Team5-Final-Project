// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GMRouterSubsystem.generated.h"

class UGMRouterSubsystem;
DECLARE_DYNAMIC_DELEGATE_TwoParams(FGameplayMessageCallback, FGameplayTag, Channel, const FInstancedStruct&, Payload);

USTRUCT(BlueprintType)
struct GMROUTER_API FGameplayMessageHandle
{
	GENERATED_BODY()
private:
	UPROPERTY() TWeakObjectPtr<UGMRouterSubsystem> Subsystem;
	UPROPERTY() FGameplayTag Channel;
	UPROPERTY() int32 ID = 0;

public:
	bool IsValid() const
	{
		return ID != 0;
	}
	void Unsubscribe();
	friend class UGMRouterSubsystem;
};
//HandleID로 콜백을 찾기 쉽도록 하는 구조체
struct FListenerEntry
{
	FGameplayMessageCallback Callback;
	int32 HandleID = 0;
};
//채널의 구독자를 관리하는 구조체
struct FChannelListenerList
{
	TArray<FListenerEntry> List;
	//최근 추가된 HandleID
	int32 NextHandleID = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*[GameplayMessage 서브시스템]
 *
 *
 * [송신]
 * 예시 구조체: FDamageMessage
 * FDamageMessage MyData;
 * MyData.Value = 100.f;
 * UGMRouterSubsystem::BroadcastMessage(GetWorld(),Tag,FInstancedStruct::Make(MyData));
 * 
 * [구독]
 *	//해당 태그를 채널로 해서 보냄(예시 : Gameplay.Event.Damage)
 *	FGameplayTag Channel = FGameplayTag::RequestGameplayTag(TEXT("Gameplay.Event.Damage"));		
 *	FGameplayMessageCallback Callback ;
 *	Callback.BindDynamic(this, &UInGameUI::OnCallbackReceived);	
 *	FGameplayMessageHandle MessageHandle=UGMRouterSubsystem::Subscribe(GetWorld(),Channel,Callback);
 *
 *
 *	
 * [수신]
 * 예시 구조체: FDamageMessage
 * void OnReceived(FGameplayTag Channel, const FInstancedStruct& Payload)
 * {
 *		//원하는 구조체로 형변환
 *		if (const FDamageMessage* Data = Payload.GetPtr<FDamageMessage>())
 *		{
 *			float FinalDamage = Data->Value;
 *		}
 * }
 * 
 * [구독 해제]
 * 	//기본적으로 객체 소멸시 자동 해제. 필요한 경우에만 사용
 * 	MessageHandle.Unsubscribe();
 * 
 * [비고]
 * 구조체는 별도의 *Type.h로 관리하는 것을 권장
 * NativeGameplayTag를 사용한다면 위의 예시 코드의 태그 부분을 수정해서 적용해야 한다.
 * 자주 Subscribe 및 BroadcastMessage를 해야한다면, 직접 이 Subsystem을 캐싱 한 후 Subscribe, BroadcastMessage를 직접 서브시스템에서 호출한다.
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UCLASS()
class GMROUTER_API UGMRouterSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	static void BroadcastMessage(const UWorld* World,const FGameplayTag& Channel, const FInstancedStruct& Payload);
	
	UFUNCTION(BlueprintCallable)
	static FGameplayMessageHandle Subscribe(const UWorld* World,const FGameplayTag& Channel, const FGameplayMessageCallback& Callback);
	
	void BroadcastMessage(const FGameplayTag& Channel, const FInstancedStruct& Payload);
	FGameplayMessageHandle Subscribe(const FGameplayTag& Channel, const FGameplayMessageCallback& Callback);
	
	UFUNCTION(BlueprintCallable)
	void Unsubscribe(const FGameplayMessageHandle& GameplayMessageHandle);
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;	
	virtual void Deinitialize() override;	
	
private:
	static UGMRouterSubsystem* GetRouterManager(const UWorld* World);
	void OnGarbageCollectionCompleted();
	
private:
	TMap<FGameplayTag, FChannelListenerList> ListenerMap;
	FDelegateHandle GarbageCollectionCompletedHandle;
	
};
