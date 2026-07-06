// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "KOLoadingUIWidget.generated.h"

class UProgressBar;
class UKOLoadingUiSubsystem;
/**
 * 
 */
UCLASS()
class KARON_API UKOLoadingUIWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	void ReceiveFinalizeSignal();
};
