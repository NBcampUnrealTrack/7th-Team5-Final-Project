// Copyright Karon Team 5. All Rights Reserved.

#include "Subsystem/KOEnergySubsystem.h"

#include "Subsystem/KOEnergyTypes.h"
#include "Engine/World.h"

namespace
{
    // 경로 압축 union-find. Producers 인덱스 단위로 커버리지 겹침을 묶는다.
    int32 FindRoot(TArray<int32>& Parent, int32 Index)
    {
        while (Parent[Index] != Index)
        {
            Parent[Index] = Parent[Parent[Index]];
            Index = Parent[Index];
        }
        return Index;
    }

    void UnionNodes(TArray<int32>& Parent, int32 A, int32 B)
    {
        const int32 RootA = FindRoot(Parent, A);
        const int32 RootB = FindRoot(Parent, B);
        if (RootA != RootB)
        {
            Parent[RootA] = RootB;
        }
    }
}

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
    bNetworkDirty = true;
}

void UKOEnergySubsystem::Deinitialize()
{
    Producers.Reset();
    Consumers.Reset();
    Networks.Reset();
    UncoveredConsumers.Reset();
    ConsumerToNetwork.Reset();
    Super::Deinitialize();
}

float UKOEnergySubsystem::GetConsumerNetworkProductionRate(const IKOEnergyConsumer* Consumer) const
{
    if (!Consumer)
    {
        return 0.f;
    }
    if (const int32* NetworkIndex = ConsumerToNetwork.Find(Consumer))
    {
        if (Networks.IsValidIndex(*NetworkIndex))
        {
            return Networks[*NetworkIndex].ProductionRate;
        }
    }
    return 0.f;
}

void UKOEnergySubsystem::RegisterProducer(IKOEnergyProducer* Producer)
{
    if (Producer)
    {
        Producers.AddUnique(Producer);
        bNetworkDirty = true;
    }
}

void UKOEnergySubsystem::RegisterConsumer(IKOEnergyConsumer* Consumer)
{
    if (Consumer)
    {
        Consumers.AddUnique(Consumer);
        bNetworkDirty = true;
    }
}

void UKOEnergySubsystem::UnregisterProducer(IKOEnergyProducer* Producer)
{
    if (Producer)
    {
        Producers.RemoveSingleSwap(Producer);
        bNetworkDirty = true;
    }
}

void UKOEnergySubsystem::UnregisterConsumer(IKOEnergyConsumer* Consumer)
{
    if (Consumer)
    {
        Consumers.RemoveSingleSwap(Consumer);
        bNetworkDirty = true;
    }
}

TStatId UKOEnergySubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UKOEnergySubsystem, STATGROUP_Tickables);
}

void UKOEnergySubsystem::RebuildNetworks()
{
    Networks.Reset();
    UncoveredConsumers.Reset();
    ConsumerToNetwork.Reset();

    const int32 ProducerCount = Producers.Num();

    // 발전기가 없으면 모든 소비자는 비커버 상태.
    if (ProducerCount == 0)
    {
        for (IKOEnergyConsumer* Consumer : Consumers)
        {
            if (Consumer)
            {
                UncoveredConsumers.Add(Consumer);
            }
        }
        return;
    }

    // 각 발전기의 커버리지 셀을 수집하면서, 같은 셀을 공유하는 발전기끼리 union.
    TArray<int32> Parent;
    Parent.SetNum(ProducerCount);
    for (int32 i = 0; i < ProducerCount; ++i)
    {
        Parent[i] = i;
    }

    TArray<TArray<FIntPoint>> Coverage;
    Coverage.SetNum(ProducerCount);

    TMap<FIntPoint, int32> CellFirstProducer;
    for (int32 i = 0; i < ProducerCount; ++i)
    {
        if (!Producers[i])
        {
            continue;
        }
        Producers[i]->GetEnergyCoverageCells(Coverage[i]);
        for (const FIntPoint& Cell : Coverage[i])
        {
            if (const int32* Existing = CellFirstProducer.Find(Cell))
            {
                UnionNodes(Parent, i, *Existing);
            }
            else
            {
                CellFirstProducer.Add(Cell, i);
            }
        }
    }

    // 루트별로 망 인덱스를 부여하고 발전기를 배치.
    TMap<int32, int32> RootToNetwork;
    for (int32 i = 0; i < ProducerCount; ++i)
    {
        if (!Producers[i])
        {
            continue;
        }
        const int32 Root = FindRoot(Parent, i);

        int32 NetworkIndex;
        if (const int32* Found = RootToNetwork.Find(Root))
        {
            NetworkIndex = *Found;
        }
        else
        {
            NetworkIndex = Networks.Add(FEnergyNetwork());
            RootToNetwork.Add(Root, NetworkIndex);
        }
        Networks[NetworkIndex].Producers.Add(Producers[i]);
    }

    // 셀 → 망 인덱스 (소비자 소속 판정용).
    TMap<FIntPoint, int32> CellToNetwork;
    for (int32 i = 0; i < ProducerCount; ++i)
    {
        if (!Producers[i])
        {
            continue;
        }
        const int32 NetworkIndex = RootToNetwork[FindRoot(Parent, i)];
        for (const FIntPoint& Cell : Coverage[i])
        {
            CellToNetwork.Add(Cell, NetworkIndex);
        }
    }

    // 소비자를 점유 셀이 속한 망에 배정. 여러 셀이 서로 다른 망이면 먼저 찾은 망에 소속.
    for (IKOEnergyConsumer* Consumer : Consumers)
    {
        if (!Consumer)
        {
            continue;
        }

        TArray<FIntPoint> Cells;
        Consumer->GetEnergyOccupiedCells(Cells);

        int32 FoundNetwork = INDEX_NONE;
        for (const FIntPoint& Cell : Cells)
        {
            if (const int32* Net = CellToNetwork.Find(Cell))
            {
                FoundNetwork = *Net;
                break;
            }
        }

        if (FoundNetwork != INDEX_NONE)
        {
            Networks[FoundNetwork].Consumers.Add(Consumer);
            ConsumerToNetwork.Add(Consumer, FoundNetwork);
        }
        else
        {
            UncoveredConsumers.Add(Consumer);
        }
    }
}

void UKOEnergySubsystem::Tick(float DeltaTime)
{
    if (DeltaTime <= 0.f)
    {
        return;
    }

    if (bNetworkDirty)
    {
        RebuildNetworks();
        bNetworkDirty = false;
    }

    // 각 망은 독립 정산: 배터리 없이 그 틱의 생산 가능량과 수요만으로 비율 분배.
    for (FEnergyNetwork& Network : Networks)
    {
        float OfferedTotal = 0.f;
        TArray<float, TInlineAllocator<16>> Offered;
        Offered.Reserve(Network.Producers.Num());
        for (IKOEnergyProducer* Producer : Network.Producers)
        {
            const float Out = Producer ? FMath::Max(0.f, Producer->GetPowerOutput(DeltaTime)) : 0.f;
            Offered.Add(Out);
            OfferedTotal += Out;
        }

        float DemandTotal = 0.f;
        TArray<float, TInlineAllocator<16>> Demands;
        Demands.Reserve(Network.Consumers.Num());
        for (IKOEnergyConsumer* Consumer : Network.Consumers)
        {
            const float Demand = Consumer ? FMath::Max(0.f, Consumer->GetPowerDemand(DeltaTime)) : 0.f;
            Demands.Add(Demand);
            DemandTotal += Demand;
        }

        const float Delivered   = FMath::Min(OfferedTotal, DemandTotal);
        const float SupplyRatio = (DemandTotal > KINDA_SMALL_NUMBER) ? (Delivered / DemandTotal) : 1.f;

        // 소비자 UI가 조회하는 망 초당 생산량 캐시.
        Network.ProductionRate = OfferedTotal / DeltaTime;

        // 발전기는 수요와 무관하게 항상 최대로 연료를 태운다(잉여 생산은 버려짐). 연료가 곧 계속 소모.
        for (int32 i = 0; i < Network.Producers.Num(); ++i)
        {
            if (Network.Producers[i])
            {
                Network.Producers[i]->OnPowerAccepted(Offered[i]);
            }
        }
        // 소비자는 생산 가능량 한도에서 수요 비율대로 공급받는다.
        for (int32 j = 0; j < Network.Consumers.Num(); ++j)
        {
            if (Network.Consumers[j])
            {
                Network.Consumers[j]->OnPowerSupplied(Demands[j] * SupplyRatio, Demands[j]);
            }
        }
    }

    // 비커버 소비자는 공급 0으로 통지 → 스스로 멈춤.
    for (IKOEnergyConsumer* Consumer : UncoveredConsumers)
    {
        if (!Consumer)
        {
            continue;
        }
        const float Demand = FMath::Max(0.f, Consumer->GetPowerDemand(DeltaTime));
        Consumer->OnPowerSupplied(0.f, Demand);
    }
}
