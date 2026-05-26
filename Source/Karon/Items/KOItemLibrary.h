// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "Data/KODataTableTypes.h"
#include "Items/KOItemTypes.h"
#include "KOItemLibrary.generated.h"

class UTexture2D;
class UStaticMesh;
class UKOLoadSubsystem;

UCLASS()
class KARON_API UKOItemLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // Item 전용
    static bool                  GetItemRow(const UObject* WorldContext, FName ItemId, FKOItemRow& OutRow);
    static FGameplayTagContainer GetItemCategories(const UObject* WorldContext, FName ItemId);
    static UStaticMesh*          GetItemMesh(const UObject* WorldContext, FName ItemId);
    
    // Item - Factory 공용
    static FText        GetDisplayName(const UObject* WorldContext, EKOSlotKind Kind, FName Id);
    static int32        GetMaxStack(const UObject* WorldContext, EKOSlotKind Kind, FName Id);
    static UTexture2D*  GetIcon(const UObject* WorldContext, EKOSlotKind Kind, FName Id);
    static bool         Exists(const UObject* WorldContext, EKOSlotKind Kind, FName Id);

private:
    static const UKOLoadSubsystem* GetLoadSubsystem(const UObject* WorldContext);
};
