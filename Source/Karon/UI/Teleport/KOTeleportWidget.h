// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/KOActivatableWidget.h"
#include "KOTeleportWidget.generated.h"

class UKOTeleportSubsystem;
class UKOBonfireEntryObject;
class UKOUISubsystem;
class UCommonButtonBase;
class UCommonTextBlock;
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
	
	void RefreshList();
	
	void BindEvents();
	
	void RemoveEvents();
	
	UFUNCTION()
	void HandleSelectionChanged(UObject* SelectedItem);
	
	UFUNCTION()
	void HandleMoveClicked();
	
	UFUNCTION()
	void HandleCancelClicked();
		
private:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonListView> BonfireListView;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonTextBlock> LocationName;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> MoveButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> CancelButton;

	UPROPERTY()
	TObjectPtr<UKOTeleportSubsystem> CachedTeleportSubsystem;
	
	UPROPERTY()
	TObjectPtr<UKOUISubsystem> CachedUISubsystem;
	
	UPROPERTY()
	TObjectPtr<UKOBonfireEntryObject> CurrentSelectedEntry;
};
