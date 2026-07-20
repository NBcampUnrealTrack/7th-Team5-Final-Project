// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Types/SlateEnums.h"
#include "UI/KOActivatableWidget.h"
#include "Game/Save/KOSaveGameOption.h"
#include "KOOptionWidget.generated.h"

class USlider;
class UCheckBox;
class UComboBoxString;
class UButton;
class UWidgetSwitcher;
class USoundMix;
class USoundClass;
class APlayerController;

UENUM(BlueprintType)
enum class EKOOptionTab : uint8
{
	Graphic,
	Sound,
	Other
};

/**
 * 옵션 팝업 위젯. UISubsystem을 통해 UI.Widget.Option 태그로 열린다.
 *
 * BP에서 해야 할 작업:
 *   1. SoundMix, SC_* 에 프로젝트 SoundClass 에셋을 지정.
 *   2. 아래 BindWidget 이름과 동일한 UMG 위젯을 배치.
 *      (ComboBox_OverallQuality는 그래픽 옵션 일괄 설정용 콤보박스로 선택 사항)
 *   3. KOUISettings WidgetMap에 UI.Widget.Option → (Layer.GameMenu, WBP_OptionWidget) 등록.
 */
UCLASS()
class KARON_API UKOOptionWidget : public UKOActivatableWidget
{
	GENERATED_BODY()

public:
	UKOOptionWidget();

	/** 저장값으로 UI를 갱신하고, 곧바로 실제 사운드/그래픽스 출력에도 반영 (팝업이 열릴 때 자동 호출됨) */
	UFUNCTION(BlueprintCallable, Category = "KO|Option")
	void LoadAndRefreshUI();

	/** Graphic/Sound 탭 전환 */
	UFUNCTION(BlueprintCallable, Category = "KO|Option")
	void SetActiveTab(EKOOptionTab Tab);

	/**
	 * 화면에 띄우지 않고, 저장된 옵션 값을 실제 SoundMix/GameUserSettings에 동기화.
	 * 게임 시작 시점(타이틀/플레이어 컨트롤러 BeginPlay)에 호출해 "설정은 저장돼 있는데 출력이
	 * 이전 값(기본값)으로 나오는" 문제를 막는다.
	 */
	UFUNCTION(BlueprintCallable, Category = "KO|Option")
	static void SyncSavedOptionsToRuntime(APlayerController* OwningPlayer);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;

protected:
	// ---- 탭 스위처 ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> ContentSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Graphic;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Sound;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Other;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Return;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Close;

protected:
	// ---- SoundClass 참조 (BP 기본값에서 에셋 지정) ----
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundMix> SoundMix;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundClass> SC_Master;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundClass> SC_BGM;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundClass> SC_SE;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundClass> SC_UI;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound")
	TObjectPtr<USoundClass> SC_Environment;

	// ---- 음향 위젯 ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> Slider_Master;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> Slider_BGM;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> Slider_SE;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> Slider_UI;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> Slider_Environment;

	// ---- 그래픽 위젯 ----
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> ComboBox_Resolution;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<USlider> Slider_ResolutionScale;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> CheckBox_Fullscreen;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> ComboBox_FrameLimit;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> CheckBox_VSync;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> ComboBox_ShadowQuality;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> ComboBox_ViewDistanceQuality;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> ComboBox_AntiAliasingQuality;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> ComboBox_TextureQuality;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> ComboBox_EffectsQuality;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> ComboBox_ShadingQuality;

	/** 그래픽 옵션 일괄 설정. 낮음/보통/높음/최고 중 하나를 고르면 아래 세부 항목에 모두 적용되고,
	 *  세부 항목을 개별로 바꾸면 자동으로 "사용자 설정"으로 표시된다. (선택 사항: BP에 없으면 무시됨) */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UComboBoxString> ComboBox_OverallQuality;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Apply;

	/** 모든 항목을 기본값으로 되돌리고 즉시 적용 */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_Reset;
	
	// 언어 선택 콤보박스
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UComboBoxString> ComboBox_CultureLanguage;

private:
	void PopulateComboBoxes();

	void RefreshSoundUI(const FKOSoundOptions& Sound);
	void RefreshGraphicsUI(const FKOGraphicsOptions& Graphics);
	void RefreshOtherUI(const int32& LanguageOption);

	/** 세부 품질 항목 6개가 모두 같은 값이면 그 값으로, 아니면 "사용자 설정"으로 ComboBox_OverallQuality를 갱신 */
	void RefreshOverallQualityUI(const FKOGraphicsOptions& Graphics);

	void ApplySoundOptions(const FKOSoundOptions& Sound);
	void ApplyGraphicsOptions(const FKOGraphicsOptions& Graphics);
	void ApplySingleSoundClass(USoundClass* SoundClass, float Volume) const;
	void ApplyCultureLanguageOptions(const int32& LanguageOption);

	FKOSoundOptions GatherSoundFromUI() const;
	FKOGraphicsOptions GatherGraphicsFromUI() const;
	const int32 GatherCultureLanguageFromUI() const;

	int32 ResolutionToIndex(FIntPoint InResolution) const;
	FIntPoint IndexToResolution(int32 Index) const;

	int32 FrameLimitToIndex(int32 InFrameLimit) const;
	int32 IndexToFrameLimit(int32 Index) const;

	UFUNCTION()
	void HandleApplyClicked();

	UFUNCTION()
	void HandleResetClicked();

	UFUNCTION()
	void HandleGraphicTabClicked();

	UFUNCTION()
	void HandleSoundTabClicked();
	
	UFUNCTION()
	void HandleOtherTabClicked();
	
	UFUNCTION()
	void ReturnEscape();
	
	UFUNCTION()
	void HandleCloseClicked();
	
	// Language 변경시 적용
	UFUNCTION()
	void HandleCultureLanguageChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	/** ComboBox_OverallQuality에서 프리셋을 고르면 세부 품질 항목 6개에 그대로 적용 */
	UFUNCTION()
	void HandleOverallQualityChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	/** 세부 품질 항목이 개별로 바뀌면 ComboBox_OverallQuality 표시를 갱신 (일치 시 프리셋명, 아니면 "사용자 설정") */
	UFUNCTION()
	void HandleQualityOptionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	/** 사운드 슬라이더는 그래픽과 달리 Apply 없이도 팝업이 열려있는 동안 바로 미리듣기로 반영된다. */
	UFUNCTION()
	void HandleSoundSliderChanged(float Value);

	static const TArray<FIntPoint> SupportedResolutions;
	static const TArray<int32> SupportedFrameLimits;
	static const TArray<FString> QualityLabels;
	static const FString CustomQualityLabel;
	static const TArray<FString> CultureLabels;

	/** true인 동안은 콤보박스 OnSelectionChanged가 프로그램적 변경을 사용자 입력으로 오인해 재귀 갱신하지 않도록 막는다 */
	bool bSuppressQualitySync = false;

	/** true인 동안은 슬라이더 OnValueChanged(SetValue로 인한 프로그램적 갱신)가 미리듣기를 다시 트리거하지 않도록 막는다 */
	bool bSuppressSoundPreview = false;

	/** 마지막으로 저장(Apply/Reset)된 사운드 값. Apply 없이 팝업이 닫히면 이 값으로 실제 출력을 되돌린다. */
	FKOSoundOptions LastSavedSound;
};