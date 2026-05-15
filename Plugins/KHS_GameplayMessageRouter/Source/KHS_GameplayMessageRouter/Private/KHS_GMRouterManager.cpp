// Fill out your copyright notice in the Description page of Project Settings.

#include "KHS_GMRouterManager.h"

void UKHS_GMRouterManager::BroadcastMessage(const FGameplayTag Channel, const FInstancedStruct& Payload)
{
	if (auto Callbacks = ListenerMap.Find(Channel))
	{
		for (const FGameplayMessageCallback& MessageDelegate : *Callbacks)
		{
			MessageDelegate.ExecuteIfBound(Channel, Payload);
		}
	}
}

void UKHS_GMRouterManager::SubscribeToMessage(const FGameplayTag Channel, const FGameplayMessageCallback& Callback)
{
	ListenerMap.FindOrAdd(Channel).Add(Callback);
}

void UKHS_GMRouterManager::Unsubscribe(const FGameplayTag Channel, const FGameplayMessageCallback& Callback)
{
	if (auto Callbacks = ListenerMap.Find(Channel))
	{
		Callbacks->Remove(Callback);
		if (Callbacks->IsEmpty())
		{
			ListenerMap.Remove(Channel);
		}
	}
}