#include "KOOverclockProgressBar.h"
#include "AbilitySystem/Tag/Event/KOGameplayTags_Event.h"
#include "Utility/Messaging/KOMessageTypes.h"

void UKOOverclockProgressBar::NativeConstruct()
{
	Super::NativeConstruct();
	
	FGameplayMessageCallback Callback ;
	Callback.BindDynamic(this, &ThisClass::ProgressBarChanged);	
	
	FGameplayMessageHandle MessageHandle = 
		UGMRouterSubsystem::Subscribe(GetWorld(),KOGameplayTags::Event_SyncOverclockProgressBar, Callback);
	
	SetVisibility(ESlateVisibility::Visible);
	UpdateRatio();
}

void UKOOverclockProgressBar::ProgressBarChanged(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	//원하는 구조체로 형변환
	const FKOOverclockProgressBarMessage* OverclockMessage = Payload.GetPtr<FKOOverclockProgressBarMessage>();
	if (!OverclockMessage) return;
	
	Ratio = OverclockMessage->Ratio;
	Alpha = Ratio == 0.f ? 0.f : Ratio == 1.f ? 1.f : Alpha; 
	
	UpdateRatio();
}
