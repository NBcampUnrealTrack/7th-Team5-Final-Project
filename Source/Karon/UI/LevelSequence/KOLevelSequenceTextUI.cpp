// Fill out your copyright notice in the Description page of Project Settings.


#include "KOLevelSequenceTextUI.h"

#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Components/TextBlock.h"
#include "Utility/Messaging/KOMessageTypes.h"

UKOLevelSequenceTextUI::UKOLevelSequenceTextUI()
{
	InputMode = EKOUIInputMode::Menu;
	bIsBackHandler = true;
}

void UKOLevelSequenceTextUI::NativeConstruct()
{
	Super::NativeConstruct();
		
	FGameplayTag Channel = KOGameplayTags::Event_LevelSequenceText;		
	FGameplayMessageCallback Callback ;
	Callback.BindDynamic(this, &ThisClass::SetText);
	FGameplayMessageHandle MessageHandle=UGMRouterSubsystem::Subscribe(GetWorld(),Channel,Callback);
	
}

void UKOLevelSequenceTextUI::SetText(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	//원하는 구조체로 형변환
	if (const FKOTextMessage* WorldNoticeMessage = Payload.GetPtr<FKOTextMessage>())
	{
		LSTextBlock->SetText(WorldNoticeMessage->InText);
	}
}
