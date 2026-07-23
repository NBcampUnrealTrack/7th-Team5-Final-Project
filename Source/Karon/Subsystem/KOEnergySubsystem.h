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

    /** 이 소비자가 속한 전력망의 직전 틱 초당 총 생산량. 비커버/미등록이면 0. */
    float GetConsumerNetworkProductionRate(const IKOEnergyConsumer* Consumer) const;

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual bool IsTickable() const override;
    virtual bool IsTickableInEditor() const override { return false; }
    virtual ETickableTickType GetTickableTickType() const override { return ETickableTickType::Conditional; }

private:
    /** 커버리지가 겹쳐 하나로 묶인 독립 전력망. 배터리 없이 매 틱 생산=소비로 즉시 정산. */
    struct FEnergyNetwork
    {
        TArray<IKOEnergyProducer*> Producers;
        TArray<IKOEnergyConsumer*> Consumers;

        /** 직전 틱 이 망의 초당 총 생산량(UI 조회용). */
        float ProductionRate = 0.f;
    };

    /** Producers/Consumers 의 현재 커버리지로 전력망(연결성분)을 다시 계산. */
    void RebuildNetworks();
    
    /** 누적된 시간을 기준으로 실제 전력망을 정산 */
    void UpdateEnergyNetworks(float StepDeltaTime);

    TArray<IKOEnergyProducer*> Producers;
    TArray<IKOEnergyConsumer*> Consumers;

    /** RebuildNetworks 결과 캐시. 어떤 발전기에도 커버되지 않은 소비자는 UncoveredConsumers 로 분리. */
    TArray<FEnergyNetwork>     Networks;
    TArray<IKOEnergyConsumer*> UncoveredConsumers;

    /** 소비자 → 소속 망 인덱스 역방향 조회. RebuildNetworks 에서 갱신. */
    TMap<const IKOEnergyConsumer*, int32> ConsumerToNetwork;

    bool bNetworkDirty = true;
    
    /** 전력 계산을 실행하는 간격. 0.1초면 초당 10회 계산 */
    float EnergyUpdateInterval = 0.1f;

    /** 이전 계산 이후 누적된 시간. */
    float EnergyUpdateAccumulator = 0.f;
};
