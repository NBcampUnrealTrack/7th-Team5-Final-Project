#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOTitleRootLayout.generated.h"

class UCommonActivatableWidgetContainerBase;
/**
 * 
 */
UCLASS()
class KARON_API UKOTitleRootLayout : public UKOActivatableWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetContainerBase> TitleLayer;

	/** 확인 다이얼로그, 옵션 팝업 등 최상위 모달 레이어. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetContainerBase> ModalLayer;
};
