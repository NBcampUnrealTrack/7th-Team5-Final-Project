// Fill out your copyright notice in the Description page of Project Settings.


#include "KOOverclockProgressBar.h"

#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Components/ProgressBar.h"
#include "Utility/Messaging/KOMessageTypes.h"

void UKOOverclockProgressBar::NativeConstruct()
{
	Super::NativeConstruct();
	FGameplayTag Channel = KOGameplayTags::Event_SyncOverclockProgressBar;		
	FGameplayMessageCallback Callback ;
	Callback.BindDynamic(this, &UKOOverclockProgressBar::ProgressBarChanged);	
	FGameplayMessageHandle MessageHandle=UGMRouterSubsystem::Subscribe(GetWorld(),Channel,Callback);
	
	SetVisibility(ESlateVisibility::Hidden);
}

void UKOOverclockProgressBar::NativeDestruct()
{
	Super::NativeDestruct();
}

void UKOOverclockProgressBar::ProgressBarChanged(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	//원하는 구조체로 형변환
	if (const FKOOverclockProgressBarMessage* OverclockMessage = Payload.GetPtr<FKOOverclockProgressBarMessage>())
	{
		if (OverclockMessage->Ratio==0.f&&GetVisibility()==ESlateVisibility::Visible)
		{
			SetVisibility(ESlateVisibility::Hidden);
		}
		if (OverclockMessage->Ratio>0.f)
		{
			if (GetVisibility()==ESlateVisibility::Hidden)
			{
				SetVisibility(ESlateVisibility::Visible);
			}
			OverclockProgressBar->SetPercent(OverclockMessage->Ratio);
		}
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().SetTimer(
				TimerHandle,
				this,
				&UKOOverclockProgressBar::OnNotChanged,
				DelayTime,
				false
				);
		}
		
		
		
		
	}
}

void UKOOverclockProgressBar::OnNotChanged()
{
	SetVisibility(ESlateVisibility::Hidden);
}
