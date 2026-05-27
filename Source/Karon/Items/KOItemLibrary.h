// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "Data/KODataTableTypes.h"
#include "Items/KOItemSlot.h"
#include "KOItemLibrary.generated.h"

class UTexture2D;
class UKOLoadSubsystem;

UCLASS()
class KARON_API UKOItemLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    // DataTable Row가 없을 때 사용하는 기본 스택 한도 — 외부에서도 동일 기준 참조 가능하도록 노출
    static constexpr int32 DefaultItemMaxStack    = 100;
    static constexpr int32 DefaultFactoryMaxStack = 1;

    // Item 전용
    static const FKOItemRow*    GetItemRow(const UObject* WorldContext, FName ItemId);

    // Factory 전용
    static const FKOFactoryRow* GetFactoryRow(const UObject* WorldContext, FName FactoryId);

    // Item - Factory 공용
    static FText        GetDisplayName(const UObject* WorldContext, EKOSlotKind Kind, FName Id);
    static int32        GetMaxStack(const UObject* WorldContext, EKOSlotKind Kind, FName Id);
    static UTexture2D*  GetIcon(const UObject* WorldContext, EKOSlotKind Kind, FName Id);
    static bool         HasRow(const UObject* WorldContext, EKOSlotKind Kind, FName Id);

private:
    static const UKOLoadSubsystem* GetLoadSubsystem(const UObject* WorldContext);
};
