#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Messaging/KOGMSInterface.h"
#include "GMRouterSubsystem.h"
#include "KOBuildQuickSlotWidget.generated.h"

class UImage;
class UTexture2D;
class UTextBlock;
class UDragDropOperation;
class UKOBuildUIComponent;
class UKOInventoryComponent;

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
	
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CountText;
	
	// 투명도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|QuickSlot")
	float NormalOpacity = 1.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build|QuickSlot")
	float DepletedOpacity = 0.25f;
	
private:
	FGameplayMessageHandle QuickSlotChangedHandle;
	FGameplayMessageHandle InventoryChangedHandle;
	
	FGameplayMessageCallback QuickSlotChangedCallback;
	FGameplayMessageCallback InventoryChangedCallback;
	
	UKOBuildUIComponent* GetBuildUIComponent() const;
	UKOInventoryComponent* GetInventoryComponent() const;

	UFUNCTION()
	void HandleQuickSlotChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload);

	UFUNCTION()
	void HandleInventoryChangedMessage(FGameplayTag Channel, const FInstancedStruct& Payload);
};