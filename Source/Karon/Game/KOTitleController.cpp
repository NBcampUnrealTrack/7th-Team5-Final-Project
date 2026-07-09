#include "KOTitleController.h"
#include "UI/KOUISubsystem.h"
#include "UI/Loading/KOLoadingUiSubsystem.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

#include "AudioDevice.h"

void AKOTitleController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	// 루트 레이아웃은 UISubsystem이 UKOUISettings::RootLayoutMap을 참조해 생성·소유한다.
	if (UKOUISubsystem* UISubsystem = UKOUISubsystem::Get(this))
	{
		UISubsystem->SetRootLayout(KOGameplayTags::UI_Layout_Title);
	}

	// 타이틀 메뉴 위젯 열기.
	UKOUISubsystem::OpenWidget(this, KOGameplayTags::UI_Widget_TitleMenu);
	
	// 로딩 창 제거 요청
	if (auto* LoadingSubsystem = GetGameInstance()->GetSubsystem<UKOLoadingUiSubsystem>())
	{
		LoadingSubsystem->HideLoadingScreen();
	}
	
	if (IsLocalController() && DefaultSoundMix)
	{
		if (FAudioDevice* AudioDevice = GetWorld()->GetAudioDeviceRaw())
		{
			AudioDevice->PushSoundMixModifier(DefaultSoundMix);
			
			UE_LOG(LogTemp, Log, TEXT("Default Sound Mix (%s) successfully pushed"),
				*DefaultSoundMix->GetName());
		}
	}
}
