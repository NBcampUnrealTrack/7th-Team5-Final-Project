// Fill out your copyright notice in the Description page of Project Settings.


#include "KOInteractionNoticeUI.h"

#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Components/TextBlock.h"
#include "Utility/Messaging/KOMessageTypes.h"

void UKOInteractionNoticeUI::NativeConstruct()
{
	Super::NativeConstruct();
	FGameplayTag Channel = KOGameplayTags::Event_Interaction;		
	FGameplayMessageCallback Callback;
	Callback.BindDynamic(this, &UKOInteractionNoticeUI::OnInteraction);	
	FGameplayMessageHandle MessageHandle=UGMRouterSubsystem::Subscribe(GetWorld(),Channel,Callback);
}

void UKOInteractionNoticeUI::NativeDestruct()
{
	Super::NativeDestruct();
}

void UKOInteractionNoticeUI::OnInteraction(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	//원하는 구조체로 형변환
	if (const FKOInteractionMessage* InteractionMessage = Payload.GetPtr<FKOInteractionMessage>())
	{
		if (!InteractionMessage->bIsActive)
		{
			SetVisibility(ESlateVisibility::Hidden);
			return;
		}
		else
		{
			SetVisibility(ESlateVisibility::Visible);
			
			FText LogText = FText::Format(
			FText::FromString(TEXT("상호작용: {0}")), 
			InteractionMessage->InteractionId
			);
			
			InteractionTextBlock->SetText(LogText);
		}
	}
}
