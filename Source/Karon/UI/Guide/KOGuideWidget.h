
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOGuideWidget.generated.h"

class UMediaPlayer;
class UImage;
class UButton;
class UTextBlock;

UCLASS()
class KARON_API UKOGuideWidget : public UKOActivatableWidget
{
	GENERATED_BODY()
	
public:
	UKOGuideWidget();
	
	virtual void NativeOnInitialized() override;
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	
public:
	// 가이드 내용 콜백
	UFUNCTION(BlueprintCallable)
	void GetGuideContent(FGameplayTag Channel, const FInstancedStruct& Payload);
	
private:
	UFUNCTION()
	void HandleCloseClicked();
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Close;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TitleTextBlock;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ScriptTextBlock;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> TutorialVideo;
	
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UMediaPlayer> TutorialMediaPlayer;
};
