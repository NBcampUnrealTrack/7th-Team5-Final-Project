
#include "KOGuideWidget.h"
#include "CommonButtonBase.h"
#include "MediaPlayer.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Components/TextBlock.h"
#include "Subsystem/KOTutorialSubsystem.h"
#include "UI/KOUISubsystem.h"

UKOGuideWidget::UKOGuideWidget()
{
	InputMode = EKOUIInputMode::Menu;
	bIsBackHandler = false;
}

void UKOGuideWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	FGameplayTag Channel = KOGameplayTags::Event_TutorialVideo;		
	FGameplayMessageCallback Callback;
	Callback.BindDynamic(this, &ThisClass::GetGuideContent);	
	FGameplayMessageHandle MessageHandle=UGMRouterSubsystem::Subscribe(GetWorld(),Channel,Callback);
}

void UKOGuideWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	if (Button_Close)
		Button_Close->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	
	
}

void UKOGuideWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
	
	if (Button_Close)
		Button_Close->OnClicked.RemoveDynamic(this, &ThisClass::HandleCloseClicked);
}

void UKOGuideWidget::GetGuideContent(FGameplayTag Channel, const FInstancedStruct& Payload)
{
	//원하는 구조체로 형변환
	if (const FKOVideoData* Data = Payload.GetPtr<FKOVideoData>())
	{
		TitleTextBlock->SetText(Data->VideoName);
		ScriptTextBlock->SetText(Data->VideoDesciption);
		TutorialMediaPlayer->SetLooping(true);
		//이미 비동기 로드되었을 경우(일반적인 상황) Get과 차이 없음
		TutorialMediaPlayer->OpenSource(Data->VideoSource.LoadSynchronous());
	}
}

void UKOGuideWidget::HandleCloseClicked()
{
	TutorialMediaPlayer->SetLooping(false);
	if (UKOTutorialSubsystem* TutorialSubsystem=UKOTutorialSubsystem::Get(this))
	{
		TutorialSubsystem->UnloadAll(this);
	}

	UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_Guide);
}
