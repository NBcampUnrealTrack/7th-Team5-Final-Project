#pragma once

#include "CoreMinimal.h"
#include "KOActivatableWidget.h"
#include "KOBuildQuickSlotBarWidget.generated.h"

class UPanelWidget;
class UKOBuildQuickSlotWidget;
class UKOBuildUIComponent;

UCLASS()
class KARON_API UKOBuildQuickSlotBarWidget : public UKOActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

public:
	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	void RebuildSlots();

private:
	UKOBuildUIComponent* GetBuildUIComponent() const;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> SlotContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build|QuickSlot")
	TSubclassOf<UKOBuildQuickSlotWidget> QuickSlotWidgetClass;
};