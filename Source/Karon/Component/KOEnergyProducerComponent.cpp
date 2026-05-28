// Copyright Karon Team 5. All Rights Reserved.

#include "Component/KOEnergyProducerComponent.h"

#include "AbilitySystem/Tag/KOGameplayTags.h"
#include "Data/KODataTableTypes.h"
#include "Subsystem/KOEnergySubsystem.h"
#include "Subsystem/KOLoadSubsystem.h"

UKOEnergyProducerComponent::UKOEnergyProducerComponent()
    : FuelCategoryTag(KOGameplayTags::Item_Category_EnergyResource)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UKOEnergyProducerComponent::BeginPlay()
{
    Super::BeginPlay();
    if (UKOEnergySubsystem* Energy = UKOEnergySubsystem::Get(this))
    {
        Energy->RegisterProducer(this);
    }
}

void UKOEnergyProducerComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if (UKOEnergySubsystem* Energy = UKOEnergySubsystem::Get(this))
    {
        Energy->UnregisterProducer(this);
    }
    Super::EndPlay(Reason);
}

int32 UKOEnergyProducerComponent::TryInsertFuel(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return Count;
    }

    if (!FuelCategoryTag.IsValid())
    {
        return Count;
    }

    const UKOLoadSubsystem* LoadSub = UKOLoadSubsystem::Get(this);
    const FKOItemRow* Row = LoadSub ? LoadSub->FindItemRow(ItemId) : nullptr;
    if (!Row || !Row->Categories.HasTag(FuelCategoryTag))
    {
        return Count;
    }

    const int32 Space = FMath::Max(0, MaxFuelBuffer - FuelInBuffer);
    const int32 ToAdd = FMath::Min(Space, Count);
    if (ToAdd > 0)
    {
        FuelInBuffer += ToAdd;
        FuelItemId    = ItemId;
    }
    return Count - ToAdd;
}

int32 UKOEnergyProducerComponent::TryExtractFuel(int32 Count)
{
    if (Count <= 0 || FuelInBuffer <= 0 || FuelItemId.IsNone())
    {
        return 0;
    }
    const int32 Taken = FMath::Min(FuelInBuffer, Count);
    FuelInBuffer -= Taken;
    if (FuelInBuffer <= 0)
    {
        FuelInBuffer = 0;
        FuelDebt     = 0.f;
        FuelItemId   = NAME_None;
    }
    return Taken;
}

void UKOEnergyProducerComponent::RestoreFuelBuffer(FName ItemId, int32 Count)
{
    if (ItemId.IsNone() || Count <= 0)
    {
        return;
    }
    FuelInBuffer += Count;
    FuelItemId    = ItemId;
}

float UKOEnergyProducerComponent::GetPowerOutput(float DeltaSeconds) const
{
    if (DeltaSeconds <= 0.f || PowerPerFuelUnit <= 0.f || BurnRatePerSecond <= 0.f)
    {
        return 0.f;
    }
    
    const float AvailableFuelUnits = static_cast<float>(FuelInBuffer) - FuelDebt;
    if (AvailableFuelUnits <= 0.f)
    {
        return 0.f;
    }

    const float DesiredBurn = BurnRatePerSecond * DeltaSeconds;
    const float ActualBurn  = FMath::Min(DesiredBurn, AvailableFuelUnits);
    return ActualBurn * PowerPerFuelUnit;
}

void UKOEnergyProducerComponent::OnPowerAccepted(float Amount)
{
    if (Amount <= 0.f || PowerPerFuelUnit <= 0.f)
    {
        return;
    }

    const float FuelConsumed = Amount / PowerPerFuelUnit;
    FuelDebt += FuelConsumed;
    
    const int32 WholeUnits = FMath::FloorToInt(FuelDebt);
    if (WholeUnits > 0)
    {
        const int32 ToRemove = FMath::Min(WholeUnits, FuelInBuffer);
        FuelInBuffer -= ToRemove;
        FuelDebt     -= static_cast<float>(ToRemove);
    }

    if (FuelInBuffer <= 0)
    {
        FuelInBuffer = 0;
        FuelDebt     = 0.f;
        FuelItemId   = NAME_None;
    }
}
