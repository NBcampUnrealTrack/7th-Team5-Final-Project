// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOTeleportWidget.generated.h"

class UKOTeleportSubsystem;
class UCommonButtonBase;
class UCommonListView;

/**
 * Teleport팝업 용 본체
 */
UCLASS()
class KARON_API UKOTeleportWidget : public UKOActivatableWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnActivated() override;
	//virtual void NativeOnDeactivated() override;
	
	void RefreshList();
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonListView> BonfireListView;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> MoveButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CancelButton;
	
private:
	UPROPERTY()
	TObjectPtr<UKOTeleportSubsystem> CachedTeleportSubsystem;
};
