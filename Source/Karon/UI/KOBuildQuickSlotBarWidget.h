#pragma once

#include "CoreMinimal.h"
#include "KOActivatableWidget.h"
#include "Messaging/KOGMSInterface.h"
#include "GMRouterSubsystem.h"
#include "Component/KOGridBuildComponent.h"
#include "KOBuildQuickSlotBarWidget.generated.h"

class UPanelWidget;
class UImage;
class UKOBuildQuickSlotWidget;
class UKOBuildUIComponent;

UCLASS()
class KARON_API UKOBuildQuickSlotBarWidget : public UKOActivatableWidget
{
	GENERATED_BODY()

public:
	UKOBuildQuickSlotBarWidget();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Back(ESC) 입력 시 건설 메뉴 전체를 종료한다(IMC/그리드/퀵슬롯바 일괄 정리). */
	virtual bool NativeOnHandleBackAction() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	void RebuildSlots();
	
	UFUNCTION(BlueprintCallable, Category = "Build|Mode")
	void SetBuildModeFrame(EKOGridBuildMode InMode);

private:
	UKOBuildUIComponent* GetBuildUIComponent() const;
	UKOGridBuildComponent* GetGridBuildComponent() const;
	
	void SetModeBorderColor(const FLinearColor& InColor);
	void SetModeBorderVisible(bool bVisible);

	UFUNCTION()
	void HandleBuildModeChangedMessage(
		FGameplayTag Channel,
		const FInstancedStruct& Payload
	);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> SlotContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build|QuickSlot")
	TSubclassOf<UKOBuildQuickSlotWidget> QuickSlotWidgetClass;
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> TopModeBorderImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> BottomModeBorderImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> LeftModeBorderImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> RightModeBorderImage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build|Mode")
	FLinearColor BuildModeBorderColor = FLinearColor(0.0f, 0.6f, 1.0f, 0.9f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build|Mode")
	FLinearColor DestroyModeBorderColor = FLinearColor(1.0f, 0.1f, 0.05f, 0.9f);

private:
	FGameplayMessageCallback BuildModeChangedCallback;
	FGameplayMessageHandle BuildModeChangedHandle;
};