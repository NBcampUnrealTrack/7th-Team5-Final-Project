// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "KOEnergySubsystem.generated.h"

class IKOEnergyProducer;
class IKOEnergyConsumer;


UCLASS()
class KARON_API UKOEnergySubsystem : public UWorldSubsystem, public FTickableGameObject
{
    GENERATED_BODY()

// Subsystem에서 Tick 사용시 FTickableGameObject Interface 사용
public:
    static UKOEnergySubsystem* Get(const UObject* WorldContext);
    
    void RegisterProducer(IKOEnergyProducer* Producer);
    void RegisterConsumer(IKOEnergyConsumer* Consumer);
    void UnregisterProducer(IKOEnergyProducer* Producer);
    void UnregisterConsumer(IKOEnergyConsumer* Consumer);
    
    float GetStoredEnergy()    const { return StoredEnergy; }
    float GetCapacity()        const { return Capacity; }
    float GetLastProduction()  const { return LastProductionRate; }
    float GetLastDemand()      const { return LastDemandRate; }
    float GetLastSupplyRatio() const { return LastSupplyRatio; }

    void SetCapacity(float NewCapacity);
    
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override { return !IsTemplate(); }
    virtual bool IsTickableInEditor() const override { return false; }
    virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }

private:
    float Capacity     = 1000.f;
    float StoredEnergy = 0.f;
    
    float LastProductionRate = 0.f;
    float LastDemandRate     = 0.f;
    float LastSupplyRatio    = 1.f;
    
    TArray<IKOEnergyProducer*> Producers;
    TArray<IKOEnergyConsumer*> Consumers;
};
