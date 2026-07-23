// Copyright Karon Team 5. All Rights Reserved.

#include "KOOptionWidget.h"
#include "UI/KOUISubsystem.h"
#include "UI/KOUISettings.h"
#include "AbilitySystem/Tag/KOGameplayTags.h"

#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/WidgetSwitcher.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"

UKOOptionWidget::UKOOptionWidget()
{
	InputMode = EKOUIInputMode::Menu;
	bIsBackHandler = true;
}

// ---- 정적 데이터 ----

const TArray<FIntPoint> UKOOptionWidget::SupportedResolutions =
{
	FIntPoint(1280,  720),
	FIntPoint(1600,  900),
	FIntPoint(1920, 1080),
	FIntPoint(2560, 1440),
	FIntPoint(3840, 2160),
};

const TArray<int32> UKOOptionWidget::SupportedFrameLimits =
{
	30, 60, 90, 120, 144, 165, 240, 0 /* 0 = 제한 없음 */
};

const TArray<FString> UKOOptionWidget::QualityLabels =
{
	TEXT("Low"),
	TEXT("Medium"),
	TEXT("High"),
	TEXT("Epic"),
};

const TArray<FString> UKOOptionWidget::CultureLabels =
{
	TEXT("한국어"),
	TEXT("English"),
};


const FString UKOOptionWidget::CustomQualityLabel = TEXT("Custom");

// ---- 초기화 ----

void UKOOptionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PopulateComboBoxes();

	if (Button_Apply)
	{
		Button_Apply->IsFocusable = false;
		Button_Apply->OnClicked.AddDynamic(this, &ThisClass::HandleApplyClicked);
	}

	if (Button_Reset)
	{
		Button_Reset->IsFocusable = false;
		Button_Reset->OnClicked.AddDynamic(this, &ThisClass::HandleResetClicked);
	}

	if (Button_Graphic)
	{
		Button_Graphic->IsFocusable = false;
		Button_Graphic->OnClicked.AddDynamic(this, &ThisClass::HandleGraphicTabClicked);
	}

	if (Button_Sound)
	{
		Button_Sound->IsFocusable = false;
		Button_Sound->OnClicked.AddDynamic(this, &ThisClass::HandleSoundTabClicked);
	}
	
	if (Button_Other)
	{
		Button_Other->IsFocusable = false;
		Button_Other->OnClicked.AddDynamic(this,&ThisClass::HandleOtherTabClicked);
	}
	
	if (Button_Return)
	{
		Button_Return->IsFocusable = false;
		Button_Return->OnClicked.AddDynamic(this, &ThisClass::ReturnEscape);
	}
	
	if (Button_Close)
	{
		Button_Close->IsFocusable = false;
		Button_Close->OnClicked.AddDynamic(this, &ThisClass::HandleCloseClicked);
	}

	if (ComboBox_OverallQuality)
	{
		ComboBox_OverallQuality->OnSelectionChanged.AddDynamic(this, &ThisClass::HandleOverallQualityChanged);
	}
	
	if (ComboBox_CultureLanguage)
	{
		ComboBox_CultureLanguage->OnSelectionChanged.AddDynamic(this,&ThisClass::HandleCultureLanguageChanged);
	}
	
	auto BindQualityOptionChanged = [this](UComboBoxString* Box)
	{
		if (Box)
		{
			Box->OnSelectionChanged.AddDynamic(this, &ThisClass::HandleQualityOptionChanged);
		}
	};

	BindQualityOptionChanged(ComboBox_ShadowQuality);
	BindQualityOptionChanged(ComboBox_ViewDistanceQuality);
	BindQualityOptionChanged(ComboBox_AntiAliasingQuality);
	BindQualityOptionChanged(ComboBox_TextureQuality);
	BindQualityOptionChanged(ComboBox_EffectsQuality);
	BindQualityOptionChanged(ComboBox_ShadingQuality);

	// 사운드는 그래픽과 달리 Apply 없이도 슬라이더를 움직이는 즉시 미리듣기로 반영한다.
	auto BindSoundSliderChanged = [this](USlider* Slider)
	{
		if (Slider)
		{
			Slider->OnValueChanged.AddDynamic(this, &ThisClass::HandleSoundSliderChanged);
		}
	};

	BindSoundSliderChanged(Slider_Master);
	BindSoundSliderChanged(Slider_BGM);
	BindSoundSliderChanged(Slider_SE);
	BindSoundSliderChanged(Slider_UI);
	BindSoundSliderChanged(Slider_Environment);
}

void UKOOptionWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	SetActiveTab(EKOOptionTab::Graphic);
	LoadAndRefreshUI();
}

void UKOOptionWidget::NativeOnDeactivated()
{
	// 사운드는 Apply 없이 팝업이 열려있는 동안 미리듣기로 반영되므로,
	// Apply를 누르지 않고 닫으면 마지막으로 저장된 값으로 실제 출력을 되돌린다.
	ApplySoundOptions(LastSavedSound);

	Super::NativeOnDeactivated();
}

void UKOOptionWidget::SetActiveTab(EKOOptionTab Tab)
{
	if (ContentSwitcher == nullptr)
	{
		return;
	}

	int32 Index = 0;

	switch (Tab)
	{
	case EKOOptionTab::Graphic:
		Index = 0;
		break;

	case EKOOptionTab::Sound:
		Index = 1;
		break;
		
	case EKOOptionTab::Other:
		Index = 2;
		break;

	default:
		Index = 0;
		break;
	}

	ContentSwitcher->SetActiveWidgetIndex(Index);
}

void UKOOptionWidget::PopulateComboBoxes()
{
	if (ComboBox_Resolution)
	{
		ComboBox_Resolution->ClearOptions();
		for (const FIntPoint& Res : SupportedResolutions)
		{
			ComboBox_Resolution->AddOption(FString::Printf(TEXT("%d x %d"), Res.X, Res.Y));
		}
	}

	if (ComboBox_FrameLimit)
	{
		ComboBox_FrameLimit->ClearOptions();
		for (int32 Limit : SupportedFrameLimits)
		{
			ComboBox_FrameLimit->AddOption(Limit == 0 ? TEXT("No Limit") : FString::FromInt(Limit));
		}
	}
	
	if (ComboBox_CultureLanguage)
	{
		ComboBox_CultureLanguage->ClearOptions();
		for (const FString& Label : CultureLabels)
		{
			ComboBox_CultureLanguage->AddOption(Label);
		}
	}

	auto PopulateQuality = [&](UComboBoxString* Box)
	{
		if (!Box) return;
		Box->ClearOptions();
		for (const FString& Label : QualityLabels)
		{
			Box->AddOption(Label);
		}
	};

	PopulateQuality(ComboBox_ShadowQuality);
	PopulateQuality(ComboBox_ViewDistanceQuality);
	PopulateQuality(ComboBox_AntiAliasingQuality);
	PopulateQuality(ComboBox_TextureQuality);
	PopulateQuality(ComboBox_EffectsQuality);
	PopulateQuality(ComboBox_ShadingQuality);

	if (ComboBox_OverallQuality)
	{
		ComboBox_OverallQuality->ClearOptions();
		for (const FString& Label : QualityLabels)
		{
			ComboBox_OverallQuality->AddOption(Label);
		}
		// 세부 항목이 서로 다른 값일 때 표시되는 항목. 사용자가 직접 골라도 별도 프리셋 값이 없어 적용되지 않는다.
		ComboBox_OverallQuality->AddOption(CustomQualityLabel);
	}
}

// ---- 로드 & UI 갱신 ----

void UKOOptionWidget::LoadAndRefreshUI()
{
	UKOSaveGameOption* Data = UKOSaveGameOption::LoadOrCreate(this);
	if (Data == nullptr)
	{
		return;
	}

	RefreshSoundUI(Data->Sound);
	RefreshGraphicsUI(Data->Graphics);
	RefreshOtherUI(Data->LanguageOption);

	// UI(슬라이더/콤보박스)만 갱신하고 끝내면 실제 출력은 저장된 값과 어긋난 채로 남는다.
	// (Apply를 눌러야만 SoundMix/GameUserSettings에 반영되던 문제) 로드 시점에도 곧바로 실제 출력에 적용한다.
	ApplySoundOptions(Data->Sound);
	ApplyGraphicsOptions(Data->Graphics);
	ApplyCultureLanguageOptions(Data->LanguageOption);

	// Apply 없이 팝업을 닫았을 때 되돌아갈 기준값
	LastSavedSound = Data->Sound;
}

void UKOOptionWidget::SyncSavedOptionsToRuntime(APlayerController* OwningPlayer)
{
	if (OwningPlayer == nullptr)
	{
		return;
	}

	const UKOUISettings* UISettings = UKOUISettings::Get();
	if (UISettings == nullptr)
	{
		return;
	}

	const FKOUIWidgetEntry* Entry = UISettings->WidgetMap.Find(KOGameplayTags::UI_Widget_Option);
	if (Entry == nullptr || Entry->WidgetClass.IsNull())
	{
		return;
	}

	const TSubclassOf<UCommonActivatableWidget> WidgetClass = Entry->WidgetClass.LoadSynchronous();
	if (WidgetClass == nullptr || !WidgetClass->IsChildOf(StaticClass()))
	{
		return;
	}

	// 화면에 띄우지 않고, 저장된 값을 실제 SoundMix/GameUserSettings에 반영하기 위한 임시 인스턴스.
	if (UKOOptionWidget* TempOptionWidget = CreateWidget<UKOOptionWidget>(OwningPlayer, WidgetClass))
	{
		TempOptionWidget->LoadAndRefreshUI();
	}
}

void UKOOptionWidget::RefreshSoundUI(const FKOSoundOptions& Sound)
{
	// SetValue는 값이 바뀌면 OnValueChanged(Direct)를 발생시키므로, 아래 프로그램적 갱신 도중에는
	// HandleSoundSliderChanged가 중간값으로 미리듣기를 적용하지 않도록 막는다.
	bSuppressSoundPreview = true;
	if (Slider_Master)     Slider_Master->SetValue(Sound.Master);
	if (Slider_BGM)        Slider_BGM->SetValue(Sound.BGM);
	if (Slider_SE)         Slider_SE->SetValue(Sound.SE);
	if (Slider_UI)         Slider_UI->SetValue(Sound.UI);
	if (Slider_Environment) Slider_Environment->SetValue(Sound.Environment);
	bSuppressSoundPreview = false;
}

void UKOOptionWidget::RefreshGraphicsUI(const FKOGraphicsOptions& Graphics)
{
	if (ComboBox_Resolution)
	{
		int32 Idx = ResolutionToIndex(Graphics.Resolution);
		ComboBox_Resolution->SetSelectedIndex(Idx >= 0 ? Idx : 2 /* 기본: 1080p */);
	}

	if (Slider_ResolutionScale)
	{
		// 슬라이더 범위 50~100, 값을 정규화해서 저장 (50 → 0.0, 100 → 1.0)
		const float Normalized = (FMath::Clamp(Graphics.ResolutionScale, 50.0f, 100.0f) - 50.0f) / 50.0f;
		Slider_ResolutionScale->SetValue(Normalized);
	}

	if (CheckBox_Fullscreen)   CheckBox_Fullscreen->SetIsChecked(Graphics.bFullscreen);
	if (CheckBox_VSync)        CheckBox_VSync->SetIsChecked(Graphics.bVSync);

	if (ComboBox_FrameLimit)
	{
		int32 Idx = FrameLimitToIndex(Graphics.FrameLimit);
		ComboBox_FrameLimit->SetSelectedIndex(Idx >= 0 ? Idx : 1 /* 기본: 60fps */);
	}

	auto SetQuality = [](UComboBoxString* Box, int32 Value)
	{
		if (Box) Box->SetSelectedIndex(FMath::Clamp(Value, 0, 3));
	};

	// SetSelectedIndex는 OnSelectionChanged(Direct)를 발생시키므로, 아래 프로그램적 갱신 도중에는
	// HandleQualityOptionChanged가 재귀적으로 끼어들지 않도록 막는다.
	bSuppressQualitySync = true;
	SetQuality(ComboBox_ShadowQuality,        Graphics.ShadowQuality);
	SetQuality(ComboBox_ViewDistanceQuality,   Graphics.ViewDistanceQuality);
	SetQuality(ComboBox_AntiAliasingQuality,   Graphics.AntiAliasingQuality);
	SetQuality(ComboBox_TextureQuality,        Graphics.TextureQuality);
	SetQuality(ComboBox_EffectsQuality,        Graphics.EffectsQuality);
	SetQuality(ComboBox_ShadingQuality,        Graphics.ShadingQuality);
	bSuppressQualitySync = false;

	RefreshOverallQualityUI(Graphics);
}

void UKOOptionWidget::RefreshOtherUI(const int32& LanguageOption)
{
	if (ComboBox_CultureLanguage)     ComboBox_CultureLanguage->SetSelectedIndex(LanguageOption);
}

void UKOOptionWidget::RefreshOverallQualityUI(const FKOGraphicsOptions& Graphics)
{
	if (ComboBox_OverallQuality == nullptr)
	{
		return;
	}

	const int32 Values[] =
	{
		Graphics.ShadowQuality,
		Graphics.ViewDistanceQuality,
		Graphics.AntiAliasingQuality,
		Graphics.TextureQuality,
		Graphics.EffectsQuality,
		Graphics.ShadingQuality,
	};

	bool bAllSame = true;
	for (int32 Value : Values)
	{
		if (Value != Values[0])
		{
			bAllSame = false;
			break;
		}
	}

	bSuppressQualitySync = true;
	if (bAllSame && QualityLabels.IsValidIndex(Values[0]))
	{
		ComboBox_OverallQuality->SetSelectedIndex(Values[0]);
	}
	else
	{
		ComboBox_OverallQuality->SetSelectedIndex(QualityLabels.Num()); // "사용자 설정"
	}
	bSuppressQualitySync = false;
}

// ---- UI에서 값 수집 ----

FKOSoundOptions UKOOptionWidget::GatherSoundFromUI() const
{
	FKOSoundOptions Out;
	if (Slider_Master)      Out.Master      = Slider_Master->GetValue();
	if (Slider_BGM)         Out.BGM         = Slider_BGM->GetValue();
	if (Slider_SE)          Out.SE          = Slider_SE->GetValue();
	if (Slider_UI)          Out.UI          = Slider_UI->GetValue();
	if (Slider_Environment) Out.Environment = Slider_Environment->GetValue();
	return Out;
}

FKOGraphicsOptions UKOOptionWidget::GatherGraphicsFromUI() const
{
	FKOGraphicsOptions Out;

	if (ComboBox_Resolution)
	{
		Out.Resolution = IndexToResolution(ComboBox_Resolution->GetSelectedIndex());
	}

	if (Slider_ResolutionScale)
	{
		// 정규화값(0~1) → 실제 스케일(50~100)
		Out.ResolutionScale = 50.0f + Slider_ResolutionScale->GetValue() * 50.0f;
	}

	if (CheckBox_Fullscreen) Out.bFullscreen = CheckBox_Fullscreen->IsChecked();
	if (CheckBox_VSync)      Out.bVSync      = CheckBox_VSync->IsChecked();

	if (ComboBox_FrameLimit)
	{
		Out.FrameLimit = IndexToFrameLimit(ComboBox_FrameLimit->GetSelectedIndex());
	}

	auto GetQuality = [](UComboBoxString* Box, int32 Default) -> int32
	{
		return Box ? FMath::Clamp(Box->GetSelectedIndex(), 0, 3) : Default;
	};

	Out.ShadowQuality        = GetQuality(ComboBox_ShadowQuality,        2);
	Out.ViewDistanceQuality  = GetQuality(ComboBox_ViewDistanceQuality,   2);
	Out.AntiAliasingQuality  = GetQuality(ComboBox_AntiAliasingQuality,   2);
	Out.TextureQuality       = GetQuality(ComboBox_TextureQuality,        2);
	Out.EffectsQuality       = GetQuality(ComboBox_EffectsQuality,        2);
	Out.ShadingQuality       = GetQuality(ComboBox_ShadingQuality,        2);

	return Out;
}

const int32 UKOOptionWidget::GatherCultureLanguageFromUI() const
{
	if (ComboBox_CultureLanguage)
	{
		return ComboBox_CultureLanguage->GetSelectedIndex();
	}
	return 0;
}

// ---- 적용 ----

void UKOOptionWidget::ApplySingleSoundClass(USoundClass* SoundClass, float Volume) const
{
	if (SoundMix == nullptr || !SoundClass)
	{
		return;
	}
	UGameplayStatics::SetSoundMixClassOverride(
		this, SoundMix, SoundClass, Volume, /*Pitch=*/1.0f, /*FadeInTime=*/0.1f, /*bApplyToChildren=*/true);
}

void UKOOptionWidget::ApplyCultureLanguageOptions(const int32& LanguageOption)
{
	switch (LanguageOption)
	{
	case 0:
		FInternationalization::Get().SetCurrentCulture("ko");
		FTextLocalizationManager::Get().RefreshResources();
		break;
	case 1:
		FInternationalization::Get().SetCurrentCulture("en");
		FTextLocalizationManager::Get().RefreshResources();
		break;
	default:
		break;
	}
}

void UKOOptionWidget::ApplySoundOptions(const FKOSoundOptions& Sound)
{
	if (SoundMix == nullptr)
	{
		return;
	}

	UGameplayStatics::PushSoundMixModifier(this, SoundMix);

	ApplySingleSoundClass(SC_Master,      Sound.Master);
	ApplySingleSoundClass(SC_BGM,         Sound.BGM);
	ApplySingleSoundClass(SC_SE,          Sound.SE);
	ApplySingleSoundClass(SC_UI,          Sound.UI);
	ApplySingleSoundClass(SC_Environment, Sound.Environment);
}

void UKOOptionWidget::ApplyGraphicsOptions(const FKOGraphicsOptions& Graphics)
{
	UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings();
	if (!Settings)
	{
		return;
	}

	Settings->SetScreenResolution(Graphics.Resolution);
	Settings->SetFullscreenMode(Graphics.bFullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);
	Settings->SetVSyncEnabled(Graphics.bVSync);
	Settings->SetFrameRateLimit(static_cast<float>(Graphics.FrameLimit));

	// 화면 해상도 스케일: GameUserSettings는 0~1 정규화 값 사용
	const float NormalizedScale = (FMath::Clamp(Graphics.ResolutionScale, 50.0f, 100.0f) - 50.0f) / 50.0f;
	Settings->SetResolutionScaleNormalized(NormalizedScale);

	Settings->SetShadowQuality(Graphics.ShadowQuality);
	Settings->SetViewDistanceQuality(Graphics.ViewDistanceQuality);
	Settings->SetAntiAliasingQuality(Graphics.AntiAliasingQuality);
	Settings->SetTextureQuality(Graphics.TextureQuality);
	Settings->SetVisualEffectQuality(Graphics.EffectsQuality);
	Settings->SetShadingQuality(Graphics.ShadingQuality);
	
	// UI에 비노출되는 항목도 명시적으로 반영
	Settings->SetGlobalIlluminationQuality(0); 
	Settings->SetPostProcessingQuality(3);
	Settings->SetReflectionQuality(0);
	Settings->SetFoliageQuality(3);

	Settings->ApplySettings(/*bCheckForCommandLineOverrides=*/false);
}

// ---- 버튼 핸들러 ----

void UKOOptionWidget::HandleApplyClicked()
{
	const FKOSoundOptions Sound       = GatherSoundFromUI();
	const FKOGraphicsOptions Graphics = GatherGraphicsFromUI();
	const int32 CultureLanguage		  = GatherCultureLanguageFromUI();

	ApplySoundOptions(Sound);
	ApplyGraphicsOptions(Graphics);
	ApplyCultureLanguageOptions(CultureLanguage);

	UKOSaveGameOption* Data = UKOSaveGameOption::LoadOrCreate(this);
	if (Data)
	{
		Data->Sound			 = Sound;
		Data->Graphics		 = Graphics;
		Data->LanguageOption = CultureLanguage;
		UKOSaveGameOption::Save(this, Data);
	}

	LastSavedSound = Sound;
}

void UKOOptionWidget::HandleResetClicked()
{
	const FKOSoundOptions DefaultSound;
	const FKOGraphicsOptions DefaultGraphics;

	RefreshSoundUI(DefaultSound);
	RefreshGraphicsUI(DefaultGraphics);

	ApplySoundOptions(DefaultSound);
	ApplyGraphicsOptions(DefaultGraphics);

	UKOSaveGameOption* Data = Cast<UKOSaveGameOption>(
		UGameplayStatics::CreateSaveGameObject(UKOSaveGameOption::StaticClass()));
	UKOSaveGameOption::Save(this, Data);

	LastSavedSound = DefaultSound;
}

void UKOOptionWidget::HandleGraphicTabClicked()
{
	SetActiveTab(EKOOptionTab::Graphic);
}

void UKOOptionWidget::HandleSoundTabClicked()
{
	SetActiveTab(EKOOptionTab::Sound);
}

void UKOOptionWidget::HandleOtherTabClicked()
{
	SetActiveTab(EKOOptionTab::Other);
}

void UKOOptionWidget::ReturnEscape()
{
	AGameModeBase* GM=UGameplayStatics::GetGameMode(this);
	APawn* Player = UGameplayStatics::GetPlayerPawn(this,0);
	APlayerController* PC=UGameplayStatics::GetPlayerController(this,0);
	
	if (!GM||!Player||!PC)
	{
		return;
	}

	AActor* PlayerStartActor = GM->ChoosePlayerStart_Implementation(PC);
	if (PlayerStartActor)
	{
		Player->SetActorLocation(PlayerStartActor->GetActorLocation());
	}
}

void UKOOptionWidget::HandleCloseClicked()
{
	UKOUISubsystem::CloseWidget(this, KOGameplayTags::UI_Widget_Option);
}

void UKOOptionWidget::HandleCultureLanguageChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (bSuppressQualitySync)
	{
		return;
	}
	const int32 PresetIndex = CultureLabels.IndexOfByKey(SelectedItem);
	if (!CultureLabels.IsValidIndex(PresetIndex))
	{
		// "사용자 설정"은 되돌릴 고정값이 없으므로 세부 항목은 그대로 둔다.
		return;
	}

	bSuppressQualitySync = true;
	
	if (ComboBox_CultureLanguage)
	{
		ComboBox_CultureLanguage->SetSelectedIndex(PresetIndex);
	}
	bSuppressQualitySync = false;
}

void UKOOptionWidget::HandleOverallQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (bSuppressQualitySync)
	{
		return;
	}

	const int32 PresetIndex = QualityLabels.IndexOfByKey(SelectedItem);
	if (!QualityLabels.IsValidIndex(PresetIndex))
	{
		// "사용자 설정"은 되돌릴 고정값이 없으므로 세부 항목은 그대로 둔다.
		return;
	}

	bSuppressQualitySync = true;

	auto ApplyPreset = [PresetIndex](UComboBoxString* Box)
	{
		if (Box) Box->SetSelectedIndex(PresetIndex);
	};

	ApplyPreset(ComboBox_ShadowQuality);
	ApplyPreset(ComboBox_ViewDistanceQuality);
	ApplyPreset(ComboBox_AntiAliasingQuality);
	ApplyPreset(ComboBox_TextureQuality);
	ApplyPreset(ComboBox_EffectsQuality);
	ApplyPreset(ComboBox_ShadingQuality);

	bSuppressQualitySync = false;
}

void UKOOptionWidget::HandleQualityOptionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (bSuppressQualitySync)
	{
		return;
	}

	RefreshOverallQualityUI(GatherGraphicsFromUI());
}

void UKOOptionWidget::HandleSoundSliderChanged(float Value)
{
	if (bSuppressSoundPreview)
	{
		return;
	}

	// 저장하지 않고 실제 출력에만 즉시 반영(미리듣기). 저장은 Apply를 눌러야만 일어난다.
	ApplySoundOptions(GatherSoundFromUI());
}

// ---- 헬퍼: 해상도 인덱스 변환 ----

int32 UKOOptionWidget::ResolutionToIndex(FIntPoint InResolution) const
{
	return SupportedResolutions.IndexOfByPredicate(
		[&](const FIntPoint& R) { return R == InResolution; });
}

FIntPoint UKOOptionWidget::IndexToResolution(int32 Index) const
{
	if (SupportedResolutions.IsValidIndex(Index))
	{
		return SupportedResolutions[Index];
	}
	return FIntPoint(1920, 1080);
}

// ---- 헬퍼: 프레임 제한 인덱스 변환 ----

int32 UKOOptionWidget::FrameLimitToIndex(int32 InFrameLimit) const
{
	return SupportedFrameLimits.IndexOfByKey(InFrameLimit);
}

int32 UKOOptionWidget::IndexToFrameLimit(int32 Index) const
{
	if (SupportedFrameLimits.IsValidIndex(Index))
	{
		return SupportedFrameLimits[Index];
	}
	return 60;
}