#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOToastMessageWidget.generated.h"

class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKOToastMessageFinished);

UCLASS()
class KARON_API UKOToastMessageWidget : public UKOActivatableWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "KO|Toast")
	void ShowMessage(const FText& InMessage, float InDuration = -1.0f);

	UPROPERTY(BlueprintAssignable, Category = "KO|Toast")
	FKOToastMessageFinished OnToastFinished;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

protected:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> MessageText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Toast")
	float DefaultDuration = 2.0f;

private:
	void HideMessage();

	bool bMessageVisible = false;
	double HideTimeSeconds = 0.0;
};