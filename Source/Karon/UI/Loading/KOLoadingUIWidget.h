// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "KOLoadingUIWidget.generated.h"

class UProgressBar;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadingFillComplete);
/**
 * 
 */
UCLASS()
class KARON_API UKOLoadingUIWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	void OnLoadingStarted(bool bIsSeamless);
	void StartFillProgressBar();

	UPROPERTY(BlueprintAssignable)
	FOnLoadingFillComplete OnFillComplete;
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;
	
private:
	float CurrentProgress = 0.0f;
	float TargetProgress = 0.0f;
	float CurrentFillSpeed = 0.0f;
	bool bFillCompleted = false;

	void SetProgressBarPercent(float Percent);
};
