// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Items/KOItemTypes.h"
#include "KOInventoryComponent.generated.h"

class UGMRouterSubsystem;

UCLASS(ClassGroup = "KO|Inventory", meta = (BlueprintSpawnableComponent))
class KARON_API UKOInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UKOInventoryComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "KO|Inventory")
    int32 MaxSlots = 20;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "KO|Inventory")
    FGameplayTagQuery AcceptedItemsQuery;

    // 아이템 조작 API
    UFUNCTION(BlueprintCallable, Category = "KO|Inventory")
    virtual int32 TryAddItem(FName ItemId, int32 Count);
    
    UFUNCTION(BlueprintCallable, Category = "KO|Inventory")
    virtual bool TryRemoveItem(FName ItemId, int32 Count);
    
    UFUNCTION(BlueprintCallable, Category = "KO|Inventory")
    bool SplitStack(int32 SlotIndex, int32 SplitCount);
    
    UFUNCTION(BlueprintCallable, Category = "KO|Inventory")
    void MergeAllStacks();

    // 아이템 조회 API 
    UFUNCTION(BlueprintPure, Category = "KO|Inventory")
    int32 GetCountOf(FName ItemId) const;
    
    UFUNCTION(BlueprintPure, Category = "KO|Inventory")
    const TArray<FKOItemSlot>& GetSlots() const { return Slots; }

    UFUNCTION(BlueprintPure, Category = "KO|Inventory")
    bool HasEnoughItems(FName ItemId, int32 Count) const;

protected:
    UPROPERTY()
    TArray<FKOItemSlot> Slots;

    void NotifyInventoryChanged(FName ItemId, int32 PreviousCount, int32 NewCount);
    virtual bool IsItemAccepted(FName ItemId) const;
    int32 GetMaxStackForItem(FName ItemId) const;
};
