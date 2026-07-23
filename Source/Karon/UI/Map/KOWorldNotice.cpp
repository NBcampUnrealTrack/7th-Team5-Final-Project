// Fill out your copyright notice in the Description page of Project Settings.


#include "KOWorldNotice.h"

#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Components/TextBlock.h"
#include "Utility/Messaging/KOMessageTypes.h"

void UKOWorldNotice::NativeConstruct()
{
	Super::NativeConstruct();
	FGameplayTag Channel = KOGameplayTags::Event_WorldNotice;		
	FGameplayMessageCallback Callback ;
	Callback.BindDynamic(this, &ThisClass::WorldEnter);
	FGameplayMessageHandle MessageHandle=UGMRouterSubsystem::Subscribe(GetWorld(),Channel,Callback);
}

void UKOWorldNotice::NativeDestruct()
{
	Super::NativeDestruct();
}

void UKOWorldNotice::WorldEnter(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	//원하는 구조체로 형변환
	if (const FKOTextMessage* WorldNoticeMessage = Payload.GetPtr<FKOTextMessage>())
	{
		WorldNameTextBlock->SetText(WorldNoticeMessage->InText);
		FOnSetTextBlock.Broadcast();
	}
}
