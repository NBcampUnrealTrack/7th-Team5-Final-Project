// Copyright Karon Team 5. All Rights Reserved.

#include "KOOptionWidget.h"

#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/WidgetSwitcher.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"

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
	TEXT("낮음"),
	TEXT("보통"),
	TEXT("높음"),
	TEXT("최고"),
	TEXT("시네마틱"),
};

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
}

void UKOOptionWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	SetActiveTab(EKOOptionTab::Graphic);
	LoadAndRefreshUI();
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
			ComboBox_FrameLimit->AddOption(Limit == 0 ? TEXT("제한 없음") : FString::FromInt(Limit));
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
}

// ---- 로드 & UI 갱신 ----

void UKOOptionWidget::LoadAndRefreshUI()
{
	UKOSaveGameOption* Data = UKOSaveGameOption::LoadOrCreate(this);
	if (!Data)
	{
		return;
	}

	RefreshSoundUI(Data->Sound);
	RefreshGraphicsUI(Data->Graphics);

	// UI(슬라이더)만 갱신하고 끝내면 실제 오디오 출력은 저장된 값과 어긋난 채로 남는다.
	// (Apply를 눌러야만 SoundMix에 반영되던 문제) 로드 시점에도 곧바로 실제 사운드에 적용한다.
	ApplySoundOptions(Data->Sound);
}

void UKOOptionWidget::RefreshSoundUI(const FKOSoundOptions& Sound)
{
	if (Slider_Master)     Slider_Master->SetValue(Sound.Master);
	if (Slider_BGM)        Slider_BGM->SetValue(Sound.BGM);
	if (Slider_SE)         Slider_SE->SetValue(Sound.SE);
	if (Slider_UI)         Slider_UI->SetValue(Sound.UI);
	if (Slider_Environment) Slider_Environment->SetValue(Sound.Environment);
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
		if (Box) Box->SetSelectedIndex(FMath::Clamp(Value, 0, 4));
	};

	SetQuality(ComboBox_ShadowQuality,        Graphics.ShadowQuality);
	SetQuality(ComboBox_ViewDistanceQuality,   Graphics.ViewDistanceQuality);
	SetQuality(ComboBox_AntiAliasingQuality,   Graphics.AntiAliasingQuality);
	SetQuality(ComboBox_TextureQuality,        Graphics.TextureQuality);
	SetQuality(ComboBox_EffectsQuality,        Graphics.EffectsQuality);
	SetQuality(ComboBox_ShadingQuality,        Graphics.ShadingQuality);
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
		return Box ? FMath::Clamp(Box->GetSelectedIndex(), 0, 4) : Default;
	};

	Out.ShadowQuality        = GetQuality(ComboBox_ShadowQuality,        3);
	Out.ViewDistanceQuality  = GetQuality(ComboBox_ViewDistanceQuality,   3);
	Out.AntiAliasingQuality  = GetQuality(ComboBox_AntiAliasingQuality,   3);
	Out.TextureQuality       = GetQuality(ComboBox_TextureQuality,        3);
	Out.EffectsQuality       = GetQuality(ComboBox_EffectsQuality,        3);
	Out.ShadingQuality       = GetQuality(ComboBox_ShadingQuality,        3);

	return Out;
}

// ---- 적용 ----

void UKOOptionWidget::ApplySingleSoundClass(USoundClass* SoundClass, float Volume) const
{
	if (!SoundMix || !SoundClass)
	{
		return;
	}
	UGameplayStatics::SetSoundMixClassOverride(
		this, SoundMix, SoundClass, Volume, /*Pitch=*/1.0f, /*FadeInTime=*/0.1f, /*bApplyToChildren=*/true);
}

void UKOOptionWidget::ApplySoundOptions(const FKOSoundOptions& Sound)
{
	if (!SoundMix)
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

	Settings->ApplySettings(/*bCheckForCommandLineOverrides=*/false);
}

// ---- 버튼 핸들러 ----

void UKOOptionWidget::HandleApplyClicked()
{
	const FKOSoundOptions Sound       = GatherSoundFromUI();
	const FKOGraphicsOptions Graphics = GatherGraphicsFromUI();

	ApplySoundOptions(Sound);
	ApplyGraphicsOptions(Graphics);

	UKOSaveGameOption* Data = UKOSaveGameOption::LoadOrCreate(this);
	if (Data)
	{
		Data->Sound    = Sound;
		Data->Graphics = Graphics;
		UKOSaveGameOption::Save(this, Data);
	}
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
}

void UKOOptionWidget::HandleGraphicTabClicked()
{
	SetActiveTab(EKOOptionTab::Graphic);
}

void UKOOptionWidget::HandleSoundTabClicked()
{
	SetActiveTab(EKOOptionTab::Sound);
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