// Copyright Karon Team 5. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"

class IKOEnergyProducer
{
public:
    virtual ~IKOEnergyProducer() = default;

    /** 공급 가능 에너지 양 */
    virtual float GetPowerOutput(float DeltaSeconds) const = 0;
    /** 이번 틱 생산(연소)할 에너지 양. 수요와 무관하게 그만큼 연료를 차감한다(항상 최대 연소). */
    virtual void OnPowerAccepted(float Amount) = 0;

    /** 이 발전기가 전력을 공급하는 그리드 셀 집합(커버리지). 전력망 구성에 사용. */
    virtual void GetEnergyCoverageCells(TArray<FIntPoint>& OutCells) const = 0;
};

class IKOEnergyConsumer
{
public:
    virtual ~IKOEnergyConsumer() = default;

    /** 소비를 원하는 양 */
    virtual float GetPowerDemand(float DeltaSeconds) const = 0;

    /** 실제로 공급받은 에너지 양 */
    virtual void OnPowerSupplied(float SuppliedAmount, float RequestedAmount) = 0;

    /** 이 소비자가 점유한 그리드 셀 집합. 어느 전력망에 속하는지 판정에 사용. */
    virtual void GetEnergyOccupiedCells(TArray<FIntPoint>& OutCells) const = 0;
};
