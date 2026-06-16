// Fill out your copyright notice in the Description page of Project Settings.

#include "GMRouterSubsystem.h"

//friend class로 Unsubscribe를 호출
void FGameplayMessageHandle::Unsubscribe()
{
	if (auto* RouterManager = Subsystem.Get())
	{
		RouterManager->Unsubscribe(*this);
	}
	ID = 0;
	Subsystem.Reset();
}

void UGMRouterSubsystem::BroadcastMessage(const FGameplayTag& Channel, const FInstancedStruct& Payload)
{
	if (auto ListenerList = ListenerMap.Find(Channel))
	{
		//콜백 함수가 실행되는 중 컨테이너의 수정이 일어날 경우 크래시 발생 위험으로 Snapshot 설정
		TArray<FListenerEntry> Snapshot= ListenerList->List;
		
		for (auto& ListenerEntry : Snapshot)
		{
			ListenerEntry.Callback.ExecuteIfBound(Channel, Payload);
		}
	}
}

FGameplayMessageHandle UGMRouterSubsystem::Subscribe(const FGameplayTag& Channel, const FGameplayMessageCallback& Callback)
{
	auto& ChannelData = ListenerMap.FindOrAdd(Channel);
	//TArray 맨끝에 주소를 가져온다
	FListenerEntry& Entry = ChannelData.List.AddDefaulted_GetRef();
	Entry.Callback = Callback;
	Entry.HandleID = ++ChannelData.NextHandleID;
	
	FGameplayMessageHandle GameplayMessageHandle;
	GameplayMessageHandle.Subsystem=this;
	GameplayMessageHandle.Channel=Channel;
	GameplayMessageHandle.ID=Entry.HandleID;
	
	return GameplayMessageHandle;
}

void UGMRouterSubsystem::BroadcastMessage(const UWorld* World, const FGameplayTag& Channel, const FInstancedStruct& Payload)
{
	if (UGMRouterSubsystem* MessageSubsystem = GetRouterManager(World))
	{
		MessageSubsystem->BroadcastMessage(Channel, Payload);
	}
}


FGameplayMessageHandle UGMRouterSubsystem::Subscribe(const UWorld* World, const FGameplayTag& Channel,
	const FGameplayMessageCallback& Callback)
{
	if (UGMRouterSubsystem* MessageSubsystem = GetRouterManager(World))
	{
		return MessageSubsystem->Subscribe(Channel, Callback);
	}
	//기본값으로 초기화시킨 구조체 반환(null
	return {};
}

void UGMRouterSubsystem::Unsubscribe(const FGameplayMessageHandle& GameplayMessageHandle)
{
	if (!GameplayMessageHandle.IsValid()||!GameplayMessageHandle.Subsystem.IsValid())
	{
		return;
	}
	//컨테이너를 순회하며 HandleID만 비교
	if (auto* ChannelData = ListenerMap.Find(GameplayMessageHandle.Channel))
	{
		int32 Index=INDEX_NONE;
		for (int32 i = 0; i < ChannelData->List.Num(); ++i)
		{
			if (ChannelData->List[i].HandleID == GameplayMessageHandle.ID)
			{
				Index = i;
				break;
			}
		}
		//매칭되는게 있다면 컨테이너에서 제거
		if (Index != INDEX_NONE)
		{
			ChannelData->List.RemoveAtSwap(Index);
		}
		//TArray가 없다면 맵에서도 제거
		if (ChannelData->List.IsEmpty())
		{
			ListenerMap.Remove(GameplayMessageHandle.Channel);
		}
	}
}

void UGMRouterSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	//GC Mark-Sweep 이후 바인딩 되어있지 않은 Callback을 제거하기 위한 콜백
	GarbageCollectionCompletedHandle = FCoreUObjectDelegates::GarbageCollectComplete
	.AddUObject(this, &UGMRouterSubsystem::OnGarbageCollectionCompleted);
}

void UGMRouterSubsystem::Deinitialize()
{
	FCoreUObjectDelegates::GarbageCollectComplete.Remove(GarbageCollectionCompletedHandle);

	Super::Deinitialize();
}

UGMRouterSubsystem* UGMRouterSubsystem::GetRouterManager(const UWorld* World)
{
	if (IsValid(World)&&World->GetGameInstance())
	{
		return World->GetGameInstance()->GetSubsystem<UGMRouterSubsystem>();
	}
	return nullptr;
}

void UGMRouterSubsystem::OnGarbageCollectionCompleted()
{
	//TMap제거를 위해 Iterator로 순회
	for (auto It = ListenerMap.CreateIterator(); It; ++It)
	{
		//바인딩되어있지 않은 콜백들 TArray에서 전부 제거
		It.Value().List.RemoveAllSwap([](const FListenerEntry& CallbackStruct)
		{
			return !CallbackStruct.Callback.IsBound()||CallbackStruct.Callback.GetUObject()==nullptr;
		});
		//콜백제거후 TArray가 비었다면, 맵에서도 제거
		if (It.Value().List.IsEmpty())
		{
			It.RemoveCurrent();
		}
	}
}
