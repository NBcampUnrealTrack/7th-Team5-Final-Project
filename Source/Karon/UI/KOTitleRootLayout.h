#pragma once

#include "CoreMinimal.h"
#include "KOActivatableWidget.h"
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
};
