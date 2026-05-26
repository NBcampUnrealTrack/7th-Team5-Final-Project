#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Messaging/KOGMSInterface.h"
#include "GMRouterSubsystem.h"
#include "KOBuildQuickSlotWidget.generated.h"

class UImage;
class UDragDropOperation;
class UTexture2D;
class UKOBuildUIComponent;

UCLASS()
class KARON_API UKOBuildQuickSlotWidget : public UUserWidget, public IKOGMSInterface
{
	GENERATED_BODY()

public:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	void SetupSlot(int32 InSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Build|QuickSlot")
	void RefreshSlot();

protected:
	virtual bool NativeOnDrop(
		const FGeometry& InGeometry,
		const FDragDropEvent& InDragDropEvent,
		UDragDropOperation* InOperation
	) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|QuickSlot", meta = (ExposeOnSpawn = "true"))
	int32 SlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|QuickSlot")
	TObjectPtr<UTexture2D> EmptySlotIcon = nullptr;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> SlotIconImage;
	
private:
	FGameplayMessageHandle QuickSlotChangedHandle;

	UFUNCTION()
	void HandleQuickSlotChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload);

	UKOBuildUIComponent* GetBuildUIComponent() const;
	
	FGameplayMessageCallback Callback;
};