// Fill out your copyright notice in the Description page of Project Settings.


#include "KOEnemyDropItemBox.h"

#include "KOEnemyDropItemLogMessage.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Components/VerticalBox.h"
#include "Utility/Messaging/KOMessageTypes.h"

void UKOEnemyDropItemBox::NativeConstruct()
{
	Super::NativeConstruct();
	
	FGameplayTag Channel = KOGameplayTags::Event_DropItem;		
	FGameplayMessageCallback Callback ;
	Callback.BindDynamic(this, &UKOEnemyDropItemBox::ItemGet);	
	FGameplayMessageHandle MessageHandle=UGMRouterSubsystem::Subscribe(GetWorld(),Channel,Callback);
}

void UKOEnemyDropItemBox::NativeDestruct()
{
	Super::NativeDestruct();
}

void UKOEnemyDropItemBox::ItemGet(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	//원하는 구조체로 형변환
	if (const FKODropItemMessage* ItemMessage = Payload.GetPtr<FKODropItemMessage>())
	{
		UKOEnemyDropItemLogMessage* ItemLogMessage=CreateWidget<UKOEnemyDropItemLogMessage>(this,LogItemMessageWidget);
		ItemLogMessage->SetLogMessage(ItemMessage->ItemId,ItemMessage->Count);
		LogItemVerticalBox->AddChildToVerticalBox(ItemLogMessage);
	}
}
