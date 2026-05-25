// Copyright Karon Team 5. All Rights Reserved.

#include "Subsystem/KOEnergySubsystem.h"

#include "Subsystem/KOEnergyTypes.h"
#include "Engine/World.h"

UKOEnergySubsystem* UKOEnergySubsystem::Get(const UObject* WorldContext)
{
    if (!WorldContext)
    {
        return nullptr;
    }
    const UWorld* World = WorldContext->GetWorld();
    return World ? World->GetSubsystem<UKOEnergySubsystem>() : nullptr;
}

void UKOEnergySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    StoredEnergy = 0.f;
}

void UKOEnergySubsystem::Deinitialize()
{
    Producers.Reset();
    Consumers.Reset();
    Super::Deinitialize();
}

void UKOEnergySubsystem::RegisterProducer(IKOEnergyProducer* Producer)
{
    if (Producer)
    {
        Producers.AddUnique(Producer);
    }
}

void UKOEnergySubsystem::RegisterConsumer(IKOEnergyConsumer* Consumer)
{
    if (Consumer)
    {
        Consumers.AddUnique(Consumer);
    }
}

void UKOEnergySubsystem::UnregisterProducer(IKOEnergyProducer* Producer)
{
    if (Producer)
    {
        Producers.RemoveSingleSwap(Producer);
    }
}

void UKOEnergySubsystem::UnregisterConsumer(IKOEnergyConsumer* Consumer)
{
    if (Consumer)
    {
        Consumers.RemoveSingleSwap(Consumer);
    }
}

void UKOEnergySubsystem::SetCapacity(float NewCapacity)
{
    Capacity = FMath::Max(0.f, NewCapacity);
    StoredEnergy = FMath::Min(StoredEnergy, Capacity);
}

TStatId UKOEnergySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UKOEnergySubsystem, STATGROUP_Tickables);
}

void UKOEnergySubsystem::Tick(float DeltaTime)
{
    if (DeltaTime <= 0.f)
    {
        return;
    }

    // Phase 1 : Producer 충전
    // Offered :  Heap 할당 대신 Stack에 
    float TotalRequested = 0.f;
    TArray<float, TInlineAllocator<32>> Offered;
    Offered.Reserve(Producers.Num());

    for (IKOEnergyProducer* P : Producers)
    {
        // Interface에서 Producer의 공급 가능 에너지량 조회
        const float Out = (P ? FMath::Max(0.f, P->GetPowerOutput(DeltaTime)) : 0.f);
        Offered.Add(Out);
        TotalRequested += Out;
    }
    
    const float FreeSpace = FMath::Max(0.f, Capacity - StoredEnergy);
    const float Accepted  = FMath::Min(TotalRequested, FreeSpace);
    const float AcceptRatio = (TotalRequested > KINDA_SMALL_NUMBER) ? (Accepted / TotalRequested) : 0.f;

    // 각 Producer에 "실제로 받아간 양" 통지 → Producer 측에서 연료를 그 비율만큼만 차감.
    for (int32 i = 0; i < Producers.Num(); ++i)
    {
        if (IKOEnergyProducer* P = Producers[i])
        {
            const float Take = Offered[i] * AcceptRatio;
            if (Take > 0.f)
            {
                P->OnPowerAccepted(Take);
            }
        }
    }
    StoredEnergy += Accepted;

    // UI 용 초당 환산 
    LastProductionRate = Accepted / DeltaTime;


    // Phase 2 : Consumer 분배
    // Offered :  Heap 할당 대신 Stack에 
    float TotalDemand = 0.f;
    TArray<float, TInlineAllocator<32>> Demands;
    Demands.Reserve(Consumers.Num());

    for (IKOEnergyConsumer* C : Consumers)
    {
        const float D = (C ? FMath::Max(0.f, C->GetPowerDemand(DeltaTime)) : 0.f);
        Demands.Add(D);
        TotalDemand += D;
    }

    // 실제 공급 가능량
    const float Available = FMath::Min(StoredEnergy, TotalDemand);
    const float SupplyRatio = (TotalDemand > KINDA_SMALL_NUMBER) ? (Available / TotalDemand) : 1.f;

    for (int32 i = 0; i < Consumers.Num(); ++i)
    {
        if (IKOEnergyConsumer* C = Consumers[i])
        {
            const float Req      = Demands[i];
            const float Supplied = Req * SupplyRatio;
            C->OnPowerSupplied(Supplied, Req);
        }
    }
    
    StoredEnergy -= Available;
    StoredEnergy  = FMath::Max(0.f, StoredEnergy);

    // UI용 통계.
    LastDemandRate  = TotalDemand / DeltaTime;
    LastSupplyRatio = SupplyRatio;
}
